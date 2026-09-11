// Translated from solution.cpp.

var eps: dynamic = 1e-9;

var pi: dynamic = acos(-1.0);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (((b == 0))) ? a : gcd(b, (a % b));
}

func xabs(a: dynamic) -> dynamic
{
  return  ((a > 0)) ? a : (-a);
}

func getCost(l: dynamic, w: dynamic, h: dynamic, rl: dynamic, rw: dynamic, cost: dynamic) -> dynamic
{
  var cnt: dynamic = 1;
  var p: dynamic = (2 * ((l + w)));
  var crl: dynamic = rl;
  var pok: dynamic = 0;
  while (true)
  {
    if ((crl >= h))
    {
      crl -= h;
    } else
    {
      cnt += 1;
      crl = rl;
      crl -= h;
    }
    pok += rw;
    if ((pok >= p))
    {
      break;
    }
  }
  return (cnt * cost);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(l[i], w[i], h[i]);
      i += 1;
    }
  }
  var m: dynamic = cpp_uninitialized();
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(rl[i], rw[i], cost[i]);
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var msum: dynamic = -1;
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((rl[j] >= h[i]))
          {
            var s: dynamic = getCost(l[i], w[i], h[i], rl[j], rw[j], cost[j]);
            if (((msum == -1) || (s < msum)))
            {
              msum = s;
            }
          }
          j += 1;
        }
      }
      sum += msum;
      i += 1;
    }
  }
  write(sum, "\n");
  return 0;
}
