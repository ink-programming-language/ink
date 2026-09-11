// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var mp: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var Q: dynamic = cpp_uninitialized();
    read(Q);
    var s: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var __cpp_switch_1: dynamic = Q;
    if (__cpp_switch_1 == 0)
    {
      read(s, x);
      mp[s] = x;
      break;
    }
    else if (__cpp_switch_1 == 1)
    {
      read(s);
      write(mp[s], "\n");
      break;
    }
    else if (__cpp_switch_1 == 2)
    {
      read(s);
      mp.erase(s);
    }
  }
  return 0;
}
