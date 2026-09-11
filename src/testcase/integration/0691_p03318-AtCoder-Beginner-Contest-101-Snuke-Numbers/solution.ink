// Translated from solution.cpp.

func f(n: dynamic) -> dynamic
{
  var res: dynamic = 0;
  while ((n > 0))
  {
    res += (n % 10);
    n /= 10;
  }
  return res;
}

func g(n: dynamic) -> dynamic
{
  return (cpp_cast((n)) / f(n));
}

func main() -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var base: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < 15))
    {
      {
        var j: dynamic = 1;
        while ((j < 150))
        {
          res.push_back(((base * ((j + 1))) - 1));
          j += 1;
        }
      }
      base *= 10;
      i += 1;
    }
  }
  sort(res.begin(), res.end());
  res.erase(unique(res.begin(), res.end()), res.end());
  {
    var i: dynamic = 0;
    while ((i < res.size()))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < res.size()))
        {
          if ((g(res[i]) > g(res[j])))
          {
            res.erase((res.begin() + cpp_update(i, "--")));
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var K: dynamic = cpp_uninitialized();
  read(K);
  {
    var i: dynamic = 0;
    while ((i < K))
    {
      write(res[i], "\n");
      i += 1;
    }
  }
}
