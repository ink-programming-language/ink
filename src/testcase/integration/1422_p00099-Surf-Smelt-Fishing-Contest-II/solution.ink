// Translated from solution.cpp.

var MAXN: dynamic = 1000006;

var INF: dynamic = (1 << 29);

class RMQ
{
  var n: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_array(((4 * MAXN) - 1));
  func init(n: dynamic = MAXN, a: dynamic = T(INF, INF)) -> dynamic
  {
      n = 1;
      while ((n < n))
      {
        n *= 2;
      }
      {
        var i: dynamic = 0;
        while ((i < ((2 * n) - 1)))
        {
          dat[i] = a;
          i += 1;
        }
      }
    }
  func update(k: dynamic, a: dynamic) -> dynamic
  {
      k += (n - 1);
      dat[k] = a;
      while ((k > 0))
      {
        k = (((k - 1)) / 2);
        dat[k] = min(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func at(k: dynamic) -> dynamic
  {
      return dat[((k + n) - 1)];
    }
  func query(a: dynamic, b: dynamic, k: dynamic = 0, l: dynamic = 0, r: dynamic = 0) -> dynamic
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
        var v1: dynamic = query(a, b, ((k * 2) + 1), l, (((l + r)) / 2));
        var v2: dynamic = query(a, b, ((k * 2) + 2), (((l + r)) / 2), r);
        return min(v1, v2);
      }
    }
}

var rmq: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  while (((cin >> n) >> q))
  {
    rmq.init(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        rmq.update(i, make_pair(0, i));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < q))
      {
        var a: dynamic = cpp_uninitialized();
        var v: dynamic = cpp_uninitialized();
        read(a, v);
        a -= 1;
        var p: dynamic = rmq.at(a);
        rmq.update(a, T((p.first - v), a));
        p = rmq.query(0, n);
        write((p.second + 1), " ", (-p.first), "\n");
        i += 1;
      }
    }
  }
  return 0;
}
