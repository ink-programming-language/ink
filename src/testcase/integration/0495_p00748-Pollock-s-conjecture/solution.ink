// Translated from solution.cpp.

var MOD = (1e9 + 7);

func chmin(a: dynamic, b: dynamic)
{
  return (if ((a > b)) ((cpp_assign(a, "=", b)) || 1) else 0);
}

func main()
{
  var d: dynamic;
  var e: dynamic;
  {
    var i = 1;
    while ((i <= 200))
    {
      var m = (((i * ((i + 1))) * ((i + 2))) / 6);
      d.push_back(m);
      if ((m % 2))
      {
        e.push_back(m);
      }
      i += 1;
    }
  }
  var dp1 = cpp_construct(1000001, 1e8);
  var dp2 = cpp_construct(1000001, 1e8);
  dp1.at(0) = 0;
  dp2.at(0) = 0;
  {
    var i = 0;
    while ((i <= 1000000))
    {
      for (var j in d)
      {
        if (((i + j) <= 1e6))
        {
          chmin(dp1.at((i + j)), (dp1.at(i) + 1));
        }
      }
      for (var j in e)
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
    var n: dynamic;
    read(n);
    if ((!n))
    {
      break;
    }
    write(dp1.at(n), cpp_char(" "), dp2.at(n), "\n");
  }
}
