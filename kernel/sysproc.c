#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

//ADDED TASK1 ASS2
uint64 sys_create_channel(void) {
  int chan_idx=0;
  struct proc * p= myproc();
  for (struct channel* curr_chan= getChannelArray(); chan_idx < CHANNELS_NUM; curr_chan++) {
    acquire(&curr_chan->chan_lock);
    printf("(sys_create_chan) Acquired chan lock in named: %s\n",curr_chan->chan_lock.name);
    if (curr_chan->chan_state == UNUSED_CHAN) {
      curr_chan->chan_state=USED_CHAN;
      curr_chan->parent_proc=p; //to be used in destroy
      acquire(&p->lock);
      printf("(sys_create_chan) Acquired chan lock in named: %s\n",p->lock.name);
      p->proc_channel=chan_idx; //FOR LATER USE TO DESTROY THIS CREATED CHANNEL IN EXIT AND WHEN THIS PROC IS KILLED
      release(&p->lock);
      release(&curr_chan->chan_lock); // relrease chan lock here to prevent acquire panic/deadlock
      return chan_idx;
    }
    release(&curr_chan->chan_lock);
    chan_idx++;
  }

  //no channel is available..
  return -1;
}

uint64 sys_channel_put(void) {
  int cd;
  argint(0,&cd);
  int data_to_put;
  argint(1,&data_to_put);
  if (cd < 0 || cd > CHANNELS_NUM) {
    printf("Channel Descriptor is out of bounds!");
    return -1;
  }
  
  struct channel * desired_chan= getChannelArray()+cd; 
  acquire(&desired_chan->chan_lock);
  if (desired_chan->chan_state == UNUSED_CHAN) {
    release(&desired_chan->chan_lock);
    return -1;
  }
   //printf("(sys_put_chan) Acquired chan lock in named: %s\n",desired_chan->chan_lock.name);
  while (desired_chan->write_chan == 0) {
    //printf("put- going to sleep, put=%d\n",data_to_put);
    sleep(&(desired_chan->write_chan), &desired_chan->chan_lock);
    if (desired_chan->chan_state == UNUSED_CHAN) {
      release(&desired_chan->chan_lock);
      printf("PUT Chan was destroyed before i woke up! exiting");
      return -1;
    }
  }

  desired_chan->data=data_to_put;
  desired_chan->read_chan=1;
  desired_chan->write_chan=0;
  wakeup(&desired_chan->read_chan);

  release(&desired_chan->chan_lock);
  //printf("Put %d\n",data_to_put);
  return 0;
}

uint64 sys_channel_take(void) { 
    int cd;
    argint(0, &cd);
    uint64 data_addr;
    argaddr(1, &data_addr);

    if (cd < 0 || cd >= CHANNELS_NUM) {
    printf("Channel Descriptor is out of bounds!");
    return -1;
    }

    
  struct channel * desired_chan= getChannelArray()+cd; 
  acquire(&desired_chan->chan_lock);
  if (desired_chan->chan_state == UNUSED_CHAN) {
    release(&desired_chan->chan_lock);
    return -1;
  }
   //printf("(sys_take_chan) Acquired chan lock in named: %s\n",desired_chan->chan_lock.name);
  while (desired_chan->read_chan == 0) {
    //printf("Take - going to sleep %p\n",&(desired_chan->read_chan));
    sleep(&(desired_chan->read_chan), &desired_chan->chan_lock);
    if (desired_chan->chan_state == UNUSED_CHAN) {
      release(&desired_chan->chan_lock);
      printf("TAKE Chan was destroyed before i woke up! exiting\n");
      return -1;
    }
  }

  desired_chan->read_chan=0;
  desired_chan->write_chan=1;
  copyout(myproc()->pagetable, data_addr, (char *)&desired_chan->data, 4); //? how many bytes to copy??
  wakeup(&desired_chan->write_chan);
  //printf("Taken %d\n",desired_chan->data);
  release(&desired_chan->chan_lock);
  
  return 0;
}

uint64 sys_channel_destroy(void) { //turn the channel to UNUSED state, wake the sleepers on BOTH channels
    int cd;
    argint(0, &cd);
    return channel_destroy(cd, CALLED_FROM_USER);
}

//ADDED TASK1 ASS2


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
