// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

var MAXN = 5000;

var MAXM = (MAXN - 1);

var n: dynamic;

var head = cpp_array(MAXN);

var nxt = cpp_array((2 * MAXM));

var to = cpp_array((2 * MAXM));

var cnt = cpp_array((MAXM + 1));

var val = cpp_array((MAXN + 1), (MAXM + 1));

func go(at: dynamic, e: dynamic)
{
  cnt[e] = 0;
  val[e][0] = 0;
  {
    var x = head[at];
    while ((x != -1))
    {
      var ne = (x >> 1);
      if ((ne == e))
      {
        x = nxt[x];
        continue;
      }
      go(to[x], ne);
      {
        var i = (cnt[e] + cnt[ne]);
        while ((i >= 0))
        {
          var nval = INT_MAX;
          {
            var i1 = max(0, (i - cnt[ne]));
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
            var i1 = max(0, (i - cnt[ne]));
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

func run()
{
  scanf("%d", (&n));
  {
    var i = (0);
    while ((i < (n)))
    {
      head[i] = -1;
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < ((n - 1))))
    {
      var a: dynamic;
      var b: dynamic;
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
  var root = -1;
  {
    var i = (0);
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

func main()
{
  run();
  return 0;
}
