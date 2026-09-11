// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

var MAXN: dynamic = 5000;

var MAXM: dynamic = (MAXN - 1);

var n: dynamic = cpp_uninitialized();

var head: dynamic = cpp_array(MAXN);

var nxt: dynamic = cpp_array((2 * MAXM));

var to: dynamic = cpp_array((2 * MAXM));

var cnt: dynamic = cpp_array((MAXM + 1));

var val: dynamic = cpp_array((MAXN + 1), (MAXM + 1));

func go(at: dynamic, e: dynamic) -> dynamic
{
  cnt[e] = 0;
  val[e][0] = 0;
  {
    var x: dynamic = head[at];
    while ((x != -1))
    {
      var ne: dynamic = (x >> 1);
      if ((ne == e))
      {
        x = nxt[x];
        continue;
      }
      go(to[x], ne);
      {
        var i: dynamic = (cnt[e] + cnt[ne]);
        while ((i >= 0))
        {
          var nval: dynamic = INT_MAX;
          {
            var i1: dynamic = max(0, (i - cnt[ne]));
            while (((i1 <= i) && (i1 <= cnt[e])))
            {
              if ((((val[e][i1] != INT_MAX) && (val[ne][(i - i1)] != INT_MAX)) && ((val[e][i1] + val[ne][(i - i1)]) < nval)))
              {
                nval = (val[e][i1] + val[ne][(i - i1)]);
              }
              i1 += 1;
            }
          }
          {
            var i1: dynamic = max(0, (i - cnt[ne]));
            while (((i1 <= i) && (i1 <= cnt[e])))
            {
              if ((((val[e][i1] != INT_MAX) && (val[ne][((i1 + cnt[ne]) - i)] != INT_MAX)) && (((val[e][i1] + val[ne][((i1 + cnt[ne]) - i)]) + 1) < nval)))
              {
                nval = ((val[e][i1] + val[ne][((i1 + cnt[ne]) - i)]) + 1);
              }
              i1 += 1;
            }
          }
          val[e][i] = nval;
          i -= 1;
        }
      }
      cnt[e] += cnt[ne];
      x = nxt[x];
    }
  }
  if ((cnt[e] == 0))
  {
    cnt[e] = 1;
    val[e][0] = INT_MAX;
    val[e][1] = 0;
  }
}

func run() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      head[i] = -1;
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < ((n - 1))))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      a -= 1;
      b -= 1;
      nxt[((2 * i) + 0)] = head[a];
      head[a] = ((2 * i) + 0);
      to[((2 * i) + 0)] = b;
      nxt[((2 * i) + 1)] = head[b];
      head[b] = ((2 * i) + 1);
      to[((2 * i) + 1)] = a;
      i += 1;
    }
  }
  if ((n == 2))
  {
    printf("1\n");
    return;
  }
  var root: dynamic = -1;
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      if (((head[i] != -1) && (nxt[head[i]] != -1)))
      {
        root = i;
        break;
      }
      i += 1;
    }
  }
  assert((root != -1));
  go(root, (n - 1));
  printf("%d\n", val[(n - 1)][(cnt[(n - 1)] / 2)]);
}

func main() -> dynamic
{
  run();
  return 0;
}
