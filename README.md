# log
log for c/c++ server   <br /> 
maxline 4096B   <br /> 
support multi-thread  <br /> 
use: <br />
1: log_init <br />
2: log_info or log_debug or log_warn <br />
3: log_close <br />
part file for time like nginx send signal and call log_reopen <br />
performance: <br />
thread:1  qps: 366153  <br />
thread:4  qps: 255916  <br />
thread:10  qps: 352872  <br />
