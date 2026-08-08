// Translated from solution.cpp.

var mapii = cpp_expression("//84104971101");

func debug(a: dynamic)
{
  return cpp_expression("//84104971101048411497 - Can yo");
}

func debuga1(a: dynamic, l: dynamic, r: dynamic)
{
  cpp_macro("fto(i, l, r) cout << a[i] << \" \"; cout << endl");
}

func fdto(i: dynamic, r: dynamic, l: dynamic)
{
  cpp_macro("for(int i = (r); i >= (l); --i)");
}

func fto(i: dynamic, l: dynamic, r: dynamic)
{
  cpp_macro("for(int i = (l); i <= (r); ++i)");
}

func forit(it: dynamic, var_cpp: dynamic)
{
  cpp_macro("for(__typeof(var.begin()) it = var.begin(); it != var.end(); it++)");
}

func forrit(rit: dynamic, var_cpp: dynamic)
{
  cpp_macro("for(__typeof(var.rbegin()) rit = var.rbegin(); rit != var.rend(); rit++)");
}

var ii = cpp_expression("//841049711010");

var iii = cpp_expression("//84104971101");

var ff = cpp_expression("//841");

var ss = cpp_expression("//8410");

var mp = cpp_expression("//8410497");

var pb = cpp_expression("//8410497");

var maxN = cpp_expression("//8");

var maxM = cpp_expression("//84");

var oo = cpp_expression("//8410497110104841149");

func sz(a: dynamic)
{
  return cpp_expression("//84104971101");
}

var PI = acos(-1.0);

func fRand(fMin: dynamic, fMax: dynamic)
{
  var f = (cpp_cast(rand()) / RAND_MAX);
  return (fMin + (f * ((fMax - fMin))));
}

func min(a: dynamic, b: dynamic, c: dynamic)
{
  return min(a, min(b, c));
}

func max(a: dynamic, b: dynamic, c: dynamic)
{
  return max(a, max(b, c));
}

func add(a: dynamic, b: dynamic)
{
  a = min((a + b), oo);
}

func mul(a: dynamic, b: dynamic)
{
  if ((a == 0))
  {
    return 0;
  }
  if (((((a * b)) / a) != b))
  {
    return oo;
  }
  return min(oo, (a * b));
}

var n: dynamic;

var m: dynamic;

var z = cpp_array(maxN, maxM);

var id: dynamic;

var cnt = cpp_array(maxM);

var dp = cpp_array(maxM);

var s = cpp_array(maxN);

func main()
{
  scanf("%d%d%lld", (&n), (&m), (&id));
  fto(i, 0, (n - 1));
  read(s[i]);
  cnt[0] = 1;
  fto(i, 1, m);
  {
    fto(j, 0, (n - 1));
    {
      if ((i >= sz(s[j])))
      {
        add(cnt[i], cnt[(i - sz(s[j]))]);
      }
    }
  }
  dp[0] = 1;
  var ans: dynamic;
  fto(i, 1, m);
  {
    fto(c, cpp_char("a"), cpp_char("z"));
    {
      var sum = 0;
      fto(j, 0, (n - 1));
      {
        fto(p, max(1, ((i - sz(s[j])) + 1)), min(i, ((m - sz(s[j])) + 1)));
        {
          if (((z[p][j] == (i - p)) && (s[j][z[p][j]] == c)))
          {
            add(sum, mul(dp[(p - 1)], cnt[(m - (((p + sz(s[j])) - 1)))]));
            if ((sum == oo))
            {
              break;
            }
          }
        }
        if ((sum == oo))
        {
          break;
        }
      }
      if ((sum >= id))
      {
        ans += c;
        fto(j, 0, (n - 1));
        {
          fto(p, max(1, ((i - sz(s[j])) + 1)), i);
          {
            if (((z[p][j] == (i - p)) && (s[j][z[p][j]] == c)))
            {
              z[p][j] += 1;
            }
          }
          if (((i >= sz(s[j])) && (z[((i - sz(s[j])) + 1)][j] == sz(s[j]))))
          {
            add(dp[i], dp[(i - sz(s[j]))]);
          }
        }
        break;
      } else
      {
        id -= sum;
      }
    }
    if ((sz(ans) != i))
    {
      puts("-");
      return 0;
    }
  }
  write(ans, "\n");
  return 0;
}
