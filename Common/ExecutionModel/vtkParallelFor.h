#include "vtkMultiThreader.h"
#include "vtkNew.h"
#include "vtkMutexLock.h"
#include <assert.h>

template<class RangeType,class FunctionType>
struct ParallelForTask
{
  const RangeType& R;
  const FunctionType& F;
  ParallelForTask(const RangeType& range, const FunctionType& function): R(range),F(function){}
};

template<class RangeType,class FunctionType>
VTK_THREAD_RETURN_TYPE vtkParallelForThreadedExecute( void *arg )
{
  typedef typename RangeType::size_type RangeSizeType;

  int threadId = static_cast<vtkMultiThreader::ThreadInfo *>(arg)->ThreadID;
  int threadCount = static_cast<vtkMultiThreader::ThreadInfo *>(arg)->NumberOfThreads;

  typedef ParallelForTask<RangeType, FunctionType> TaskType;
  TaskType* task = static_cast<TaskType*>( (static_cast<vtkMultiThreader::ThreadInfo *>(arg)->UserData));

  const FunctionType& F(task->F);
  const RangeType& R(task->R);

  int dataSize = static_cast<int>(R.size());
  int chunkSize = dataSize/threadCount;
  int chunkBegin = threadId*chunkSize;
  int chunkEnd = threadId < threadCount-1? chunkBegin+chunkSize : dataSize;

  assert(chunkSize>0);
  RangeType r(static_cast<RangeSizeType>(chunkBegin),static_cast<RangeSizeType>(chunkEnd));

  F(r);
  return VTK_THREAD_RETURN_VALUE;

}

template<class RangeType, class FunctionType>
void vtkParallelFor(const RangeType& r, const FunctionType& func)
{
  vtkNew<vtkMultiThreader> threader;
  int numThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
  ParallelForTask<RangeType,FunctionType> task(r,func);

  if( ((r.end()-r.begin())/numThreads)==0)
    {
    cout<<"Not enough task. Just run it."<<endl;
    func(r);
    }
  else
    {
    cout<<"Spawn "<<numThreads<<" threads\n";
    threader->SetSingleMethod(vtkParallelForThreadedExecute<RangeType,FunctionType>, &task);
    threader->SingleMethodExecute();
    }
}
