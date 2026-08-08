// Translated from solution.cpp.

var N = 305;

var c = cpp_array(N);

var l = cpp_array(N);

func gcd(a: dynamic, b: dynamic)
{
  while (b)
  {
    a %= b;
    swap(a, b);
  }
  return a;
}

func rrand()
{
  var a = rand();
  var b = rand();
  return (a + ((b >> 16)));
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(l[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(c[i]);
      i += 1;
    }
  }
  var best = 0;
  var prices: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var value = l[i];
      var cost = c[i];
      {
        var it = prices.begin();
        while ((it != prices.end()))
        {
          var to = gcd(it->first, value);
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
