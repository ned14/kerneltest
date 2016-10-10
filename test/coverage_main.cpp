#include "auto_permute_test_kernel2.hpp"
#include <stdio.h>
#include <iostream>

#if 0
int main(void)
{
  auto r = file_create::test_file_create("foo");
  return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
  if(argc&1)
  {
    printf("%d\n", argc);
  }
  auto r = file_create::openfile("foo");
  printf("Call returns has_value=%d, has_error=%d, has_exception=%d\n", r.has_value(), r.has_error(), r.has_exception());
  std::cout << r << std::endl;
  return 0;
}
#endif

#if 0
extern "C"
{
  void __sanitizer_cov_trace_pc()
  {
    printf("arc pc=%p\n", __builtin_return_address(0));    
  }
  
  void __sanitizer_cov_trace_pc_indirect(void *callee)
  {
    printf("arc pc=%p, callee=%p\n", __builtin_return_address(0), callee);        
  }

  void __sanitizer_cov_trace_cmp1(uint8_t Arg1, uint8_t Arg2)
  {
    printf("cmp pc=%p,%u,%u\n", __builtin_return_address(0), Arg1, Arg2);    
  }

  void __sanitizer_cov_trace_cmp2(uint16_t Arg1, uint16_t Arg2)
  {
    printf("cmp pc=%p,%u,%u\n", __builtin_return_address(0), Arg1, Arg2);    
  }

  void __sanitizer_cov_trace_cmp4(uint32_t Arg1, uint32_t Arg2)
  {
    printf("cmp pc=%p,%u,%u\n", __builtin_return_address(0), Arg1, Arg2);    
  }

  void __sanitizer_cov_trace_cmp8(uint64_t Arg1, uint64_t Arg2)
  {
    printf("cmp pc=%p,%lu,%lu\n", __builtin_return_address(0), Arg1, Arg2);    
  }

  void __sanitizer_cov_trace_switch(uint64_t Val, uint64_t *Cases)
  {
    printf("swi pc=%p,%lu,{", __builtin_return_address(0), Val);
    for(uint64_t i=0; i<Cases[0]; i++)
    {
      if(i>0)
        printf(",");
      printf("%lu", Cases[2+i]);
    }
    printf("}\n");
  }

}
#endif
