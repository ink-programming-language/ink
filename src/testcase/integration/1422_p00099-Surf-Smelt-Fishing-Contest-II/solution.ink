// Translated from solution.cpp.

var MAXN = 1000006;

var INF = (1 << 29);

class RMQ
{
  var n: dynamic;
  var dat: dynamic = cpp_array(((4 * MAXN) - 1));
  func init(n: dynamic = MAXN, a: dynamic = T(INF, INF))
  {
      n = 1;
      while ((n < n))
      {
        n *= 2;
      }
      {
        var i = 0;
        while ((i < ((2 * n) - 1)))
        {
          dat[i] = a;
          i += 1;
        }
      }
    }
  func update(k: dynamic, a: dynamic)
  {
      k += (n - 1);
      dat[k] = a;
      while ((k > 0))
      {
        k = (((k - 1)) / 2);
        dat[k] = min(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func at(k: dynamic)
  {
      return dat[((k + n) - 1)];
    }
  func query(a: dynamic, b: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = 0)
  {
      if ((k == 0))
      {
        l = 0;
        r = n;
      }
      if (((r <= a) || (b <= l)))
      {
        return T(INF, INF);
      }
      if (((a <= l) && (r <= b)))
      {
        return dat[k];
      } else
      {
        var v1 = query(a, b, ((k * 2) + 1), l, (((l + r)) / 2));
        var v2 = query(a, b, ((k * 2) + 2), (((l + r)) / 2), r);
        return min(v1, v2);
      }
    }
}

var rmq: dynamic;

func main()
{
  var n: dynamic;
  var q: dynamic;
  while (((cin >> n) >> q))
  {
    rmq.init(n);
    {
      var i = 0;
      while ((i < n))
      {
        rmq.update(i, make_pair(0, i));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < q))
      {
        var a: dynamic;
        var v: dynamic;
        read(a, v);
        a -= 1;
        var p = rmq.at(a);
        rmq.update(a, T((p.first - v), a));
        p = rmq.query(0, n);
        write((p.second + 1), " ", (-p.first), "\n");
        i += 1;
      }
    }
  }
  return 0;
}
