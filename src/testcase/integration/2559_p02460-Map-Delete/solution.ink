// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var mp: dynamic;

func main()
{
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var Q: dynamic;
    read(Q);
    var s: dynamic;
    var x: dynamic;
    var __cpp_switch_1 = Q;
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
