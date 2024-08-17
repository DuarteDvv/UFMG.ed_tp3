/* Estrutura de Dados
 * Wagner Meira Jr. - 12/06/2024
 *
 * Sistema de memoria virtual
 *
 * Este codigo implementa um SMV simples, com politica
 * de gerenciamento randomica.
 *
 */

#include "LRU.h"

#include "smv.h"

#define NOPROT 0    /* not protected */
#define READPROT 1  /* protected for read */
#define WRITEPROT 2 /* protected for write */
#define VALID 4     /* data in phys memory (RAM) */
#define DIRTY 8     /* data in cache, the page has been written */
#define AVAIL 16    /* data in cache, protected for R/W */
#define READ 32     /* data in cache, protected for W */
#define DISCO 64    /* data in secondary memory */

typedef struct smvpage
{
  int status;
  void *logaddr;
  void *physaddr;
  int nacc,   // number of times it is accessed
      nvalid, // number of times it became valid
      ndirty, // number of times it became dirty
      navail, // number of times it became available
      nread,  // number of times read was allowed
      ndisk;  // number of times it is in disco
} smvpage_t, *ptr_smvpage_t;

void segv_handler(int sig, siginfo_t *sip, ucontext_t *uap);

smvpage_t pvet[NUMPAGE];

/* There are two address spaces: the logical, which is effectively used */
/* by the program, and the physical, where the data is kept in memory */
/* The logical space works as a cache for the physical space. Although */
/* both have the same size, at most one page of the logical space is */
/* used at a given time. This strategy is not efficient at all, but */
/* allow us to manage the VM system and implement replacement policies */
/* easily. */

char *physpage, *logpage;
int swap;              /* file handler of swap */
int pagesinmemory = 0; /* number of pages in memory */
LRUList lruList;

void segv_handler(int sig, siginfo_t *sip, ucontext_t *uap)
{
  unsigned int i = ((caddr_t)sip->si_addr - logpage) / PAGESIZE;
  int c, discard;
#ifdef DEBUG
  printf("page fault at page %d: %x\n", i, pvet[i].status);
#endif
  pvet[i].nacc++;

  /* do we have any pages dirty? save them */
  /* notice that we must have at most one page dirty... */
  for (c = 0; c < NUMPAGE; c++)
  {
    if (pvet[c].status & DIRTY)
    {
      pvet[c].status &= ~DIRTY;
      pvet[c].status |= VALID;
      memcpy(pvet[c].physaddr, pvet[c].logaddr, PAGESIZE);
#ifdef DEBUG
      printf("page %d was updated: %x\n", c, pvet[c].status);
#endif
      pvet[c].nvalid++;
    }
    else if ((c != i) && (pvet[c].status & (AVAIL + READ)))
    {
      pvet[c].status &= ~(AVAIL + READ);
      pvet[c].status |= VALID;
#ifdef DEBUG
      printf("page %d became valid: %x\n", c, pvet[c].status);
#endif
      pvet[c].nvalid++;
    }
  }

  /* in order to implement the replacement policy properly, we protect */
  /* all pages at every page fault */
  if (mprotect(logpage, NUMPAGE * PAGESIZE, PROT_NONE))
  {
    perror("Couldn't mprotect");
    exit(errno);
  }

  if (pvet[i].status & DISCO)
  {
    /* here comes the VM System */
    /* the proper page is loaded and its status evolves to VALID */
    /* do we have space in memory? */
    if (pagesinmemory + 1 > NUMPAGE * MEMTOSWAPRATIO)
    {
      LRUNode *discardNode = lruList.removeTail();
      discard = discardNode->index;

      if (discardNode)
      {
        lseek(swap, discard * PAGESIZE, SEEK_SET);
        write(swap, pvet[discard].physaddr, PAGESIZE);
        pvet[discard].status &= ~VALID;
        pvet[discard].status |= DISCO;
        pvet[discard].ndisk++;
        pagesinmemory--;

        delete discardNode;

#ifdef DEBUG
        printf("Discarding %d to disk (%d)\n", discard, pagesinmemory);
#endif
      }
      else
      {
        printf("ERROR: Couldn't find a page to swap out!\n");
      }
    }
    /* load new page */
    pvet[i].status &= ~DISCO;
    pvet[i].status |= VALID;
    pvet[i].nvalid++;
    lseek(swap, i * PAGESIZE, SEEK_SET);
    read(swap, pvet[i].physaddr, PAGESIZE);
    pagesinmemory++;

    lruList.addToHead(i);

#ifdef DEBUG
    printf("Loading %d to memory (%d)\n", i, pagesinmemory);
#endif
  }
  else if (pvet[i].status & VALID)
  {
    /* the page is in physical memory and we must make it avail in */
    /* cache memory, the status should change to AVAIL */
    pvet[i].status &= ~VALID;
    pvet[i].status |= AVAIL;
    pvet[i].navail++;
    if (mprotect(pvet[i].logaddr, PAGESIZE, PROT_READ | PROT_WRITE))
    {
      perror("Couldn't mprotect");
      exit(errno);
    }
    memcpy(pvet[i].logaddr, pvet[i].physaddr, PAGESIZE);
    if (mprotect(pvet[i].logaddr, PAGESIZE, PROT_NONE))
    {
      perror("Couldn't mprotect");
      exit(errno);
    }

    lruList.moveToHead(lruList.addToHead(i));
#ifdef DEBUG
    printf("page %d became avail: %x\n", i, pvet[i].status);
#endif
  }
  else if (pvet[i].status & AVAIL)
  {
    /* the page is in cache, and we are trying to access it, either */
    /* for reading or writing, and protected for R/W, unprotect it for */
    /* read and change the status to READ */
    pvet[i].status &= ~AVAIL;
    pvet[i].status |= READ;
    pvet[i].nread++;
    if (mprotect(pvet[i].logaddr, PAGESIZE, PROT_READ))
    {
      perror("Couldn't mprotect");
      exit(errno);
    }

    lruList.moveToHead(lruList.addToHead(i));
#ifdef DEBUG
    printf("page %d can be read: %x\n", i, pvet[i].status);
#endif
  }
  else if (pvet[i].status & READ)
  {
    /* if we have a fault, we should be trying to write, unprotect it  */
    /* for write and change status to DIRTY */
    pvet[i].status &= ~READ;
    pvet[i].status |= DIRTY;
    pvet[i].ndirty++;
    if (mprotect(pvet[i].logaddr, PAGESIZE, PROT_READ | PROT_WRITE))
    {
      perror("Couldn't mprotect");
      exit(errno);
    }

    lruList.moveToHead(lruList.addToHead(i));
#ifdef DEBUG
    printf("page %d can be written: %x\n", i, pvet[i].status);
#endif
  }
  else if (pvet[i].status & DIRTY)
  {
    /* this must never happen. something is wrong :-) */
#ifdef DEBUG
    printf("ERROR: page %d is already dirty: %x\n", i, pvet[i].status);
#endif
  }
}

void end_page()
{
  int j;
  for (j = 0; j < NUMPAGE; j++)
  {
    if (pvet[j].nacc)
    {
      printf("Page %d: acc %d val %d drt %d ava %d rd %d dsk %d\n",
             j, pvet[j].nacc, pvet[j].nvalid, pvet[j].ndirty,
             pvet[j].navail, pvet[j].nread, pvet[j].ndisk);
    }
    if (pvet[j].status & VALID)
    {
      lseek(swap, j * PAGESIZE, SEEK_SET);
      write(swap, pvet[j].physaddr, PAGESIZE);
      pvet[j].status &= ~VALID;
      pvet[j].status |= DISCO;
      pagesinmemory--;
    }
  }
  close(swap);
}

char *init_page(int *bytesallocated)
{
  int i;
  char swapname[30];
  char pagenull[PAGESIZE];

  // set handler

  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = (void (*)(int, siginfo_t *, void *))segv_handler;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGALRM);
  sigaddset(&sa.sa_mask, SIGIO);
  sigaction(SIGSEGV, &sa, NULL);

  printf("init_page(): allocating %d bytes\n", NUMPAGE * PAGESIZE + PAGESIZE - 1);
  physpage = (char *)malloc(NUMPAGE * PAGESIZE + PAGESIZE - 1);
  if (!physpage)
  {
    perror("Couldn't malloc(1024)");
    exit(errno);
  }
  logpage = (char *)malloc(NUMPAGE * PAGESIZE + PAGESIZE - 1);
  if (!logpage)
  {
    perror("Couldn't malloc(1024)");
    exit(errno);
  }
  *bytesallocated = NUMPAGE * PAGESIZE;

  /* Align to a multiple of PAGESIZE, assumed to be a power of two */
  physpage = (char *)(((long)physpage + PAGESIZE - 1) & ~(PAGESIZE - 1));
  logpage = (char *)(((long)logpage + PAGESIZE - 1) & ~(PAGESIZE - 1));
  memset(pagenull, 0, PAGESIZE);
  sprintf(swapname, "smvswap.%d", (int)getpid());
  swap = open(swapname, O_RDWR);
  for (i = 0; i < NUMPAGE; i++)
  {
    pvet[i].status = DISCO;
    pvet[i].logaddr = logpage + i * PAGESIZE;
    pvet[i].physaddr = physpage + i * PAGESIZE;
    write(swap, pagenull, PAGESIZE);
    pvet[i].nacc = 0;   // number of times it is accessed
    pvet[i].nvalid = 0; // number of times it became valid
    pvet[i].ndirty = 0; // number of times it became dirty
    pvet[i].navail = 0; // number of times it became available
    pvet[i].nread = 0;  // number of times read was allowed
    pvet[i].ndisk = 0;  // number of times it is in disco
  }
  if (mprotect(logpage, NUMPAGE * PAGESIZE, PROT_NONE))
  {
    perror("Couldn't mprotect logpage");
    exit(errno);
  }
  return logpage;
}
