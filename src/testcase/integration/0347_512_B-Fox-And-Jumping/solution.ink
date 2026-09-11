// Translated from solution.cpp.

var N: dynamic = 305;

var c: dynamic = cpp_array(N);

var l: dynamic = cpp_array(N);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  while (b)
  {
    a %= b;
    swap(a, b);
  }
  return a;
}

func rrand() -> dynamic
{
  var a: dynamic = rand();
  var b: dynamic = rand();
  return (a + ((b >> 16)));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(l[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(c[i]);
      i += 1;
    }
  }
  var best: dynamic = 0;
  var prices: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var value: dynamic = l[i];
      var cost: dynamic = c[i];
      {
        var it: dynamic = prices.begin();
        while ((it != prices.end()))
        {
          var to: dynamic = gcd(it->first, value);
          if (((prices.count(to) == 0) || (prices[to] > (cost + it->second))))
          {
            prices[to] = (cost + it->second);
          }
          it += 1;
        }
      }
      if (((prices.count(value) == 0) || (prices[value] > cost)))
      {
        prices[value] = cost;
      }
      i += 1;
    }
  }
  if ((prices.count(1) == 0))
  {
    write(-1, "\n");
    return 0;
  }
  write(prices[1], "\n");
  return 0;
}
