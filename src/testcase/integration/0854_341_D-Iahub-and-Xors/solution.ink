// Translated from solution.cpp.

var N: dynamic = 1002;

var M: dynamic = 123;

var Pi: dynamic = acos(-1);

var Inf: dynamic = 1e18;

var inf: dynamic = 1e9;

var mod: dynamic = (1e9 + 7);

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
}

func mult(a: dynamic, b: dynamic) -> dynamic
{
  return (((1 * a) * b) % mod);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(N, N, 2, 2);

func get(x: dynamic, y: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = x;
    while ((i > 0))
    {
      {
        var j: dynamic = y;
        while ((j > 0))
        {
          ans ^= t[(x & 1)][(y & 1)][i][j];
          j = (((j & ((j + 1)))) - 1);
        }
      }
      i = (((i & ((i + 1)))) - 1);
    }
  }
  return ans;
}

func upd(x: dynamic, y: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = x;
    while ((i <= n))
    {
      {
        var j: dynamic = y;
        while ((j <= n))
        {
          t[(x & 1)][(y & 1)][i][j] ^= v;
          j = ((j | ((j + 1))));
        }
      }
      i = ((i | ((i + 1))));
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var it: dynamic = 0;
    while ((it < m))
    {
      var t: dynamic = cpp_uninitialized();
      var x1: dynamic = cpp_uninitialized();
      var y1: dynamic = cpp_uninitialized();
      var x2: dynamic = cpp_uninitialized();
      var y2: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d%d%d%d", (&t), (&x1), (&y1), (&x2), (&y2));
      if ((t == 1))
      {
        var ans: dynamic = (((get(x2, y2) ^ get((x1 - 1), (y1 - 1))) ^ get((x1 - 1), y2)) ^ get(x2, (y1 - 1)));
        printf("%d\n", ans);
      } else
      {
        scanf("%d", (&v));
        upd(x1, y1, v);
        upd((x2 + 1), y1, v);
        upd(x1, (y2 + 1), v);
        upd((x2 + 1), (y2 + 1), v);
      }
      it += 1;
    }
  }
  return 0;
}
