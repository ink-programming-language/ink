// Translated from solution.cpp.

var eps = 1e-9;

var pi = acos(-1.0);

func gcd(a: dynamic, b: dynamic)
{
  return if (((b == 0))) a else gcd(b, (a % b));
}

func xabs(a: dynamic)
{
  return if ((a > 0)) a else (-a);
}

func getCost(l: dynamic, w: dynamic, h: dynamic, rl: dynamic, rw: dynamic, cost: dynamic)
{
  var cnt = 1;
  var p = (2 * ((l + w)));
  var crl = rl;
  var pok = 0;
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

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(l[i], w[i], h[i]);
      i += 1;
    }
  }
  var m: dynamic;
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      read(rl[i], rw[i], cost[i]);
      i += 1;
    }
  }
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var msum = -1;
      {
        var j = 0;
        while ((j < m))
        {
          if ((rl[j] >= h[i]))
          {
            var s = getCost(l[i], w[i], h[i], rl[j], rw[j], cost[j]);
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
