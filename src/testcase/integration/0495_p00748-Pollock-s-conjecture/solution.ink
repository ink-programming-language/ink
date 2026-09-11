// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((a > b)) ? ((cpp_assign(a, "=", b)) || 1) : 0);
}

func main() -> dynamic
{
  var d: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= 200))
    {
      var m: dynamic = (((i * ((i + 1))) * ((i + 2))) / 6);
      d.push_back(m);
      if ((m % 2))
      {
        e.push_back(m);
      }
      i += 1;
    }
  }
  var dp1: dynamic = cpp_construct(1000001, 1e8);
  var dp2: dynamic = cpp_construct(1000001, 1e8);
  dp1.at(0) = 0;
  dp2.at(0) = 0;
  {
    var i: dynamic = 0;
    while ((i <= 1000000))
    {
      for (var j: dynamic in d)
      {
        if (((i + j) <= 1e6))
        {
          chmin(dp1.at((i + j)), (dp1.at(i) + 1));
        }
      }
      for (var j: dynamic in e)
      {
        if (((i + j) <= 1e6))
        {
          chmin(dp2.at((i + j)), (dp2.at(i) + 1));
        }
      }
      i += 1;
    }
  }
  while (1)
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    if ((!n))
    {
      break;
    }
    write(dp1.at(n), cpp_char(" "), dp2.at(n), "\n");
  }
}
