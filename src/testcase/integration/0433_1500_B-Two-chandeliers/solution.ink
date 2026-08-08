// Translated from solution.cpp.

var USE_MATH_DEFINES = cpp_expression("#def");

var pb = cpp_expression("#define _");

var en = cpp_expression("#def");

func forn(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0;i<n;i++)");
}

func for0(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0;i<n;i++)");
}

func all(x: dynamic)
{
  return cpp_expression("#define _USE_MATH_DEF");
}

func rall(x: dynamic)
{
  return cpp_expression("#define _USE_MATH_DEFIN");
}

var vec = cpp_expression("#defin");

var pii = cpp_expression("#define _USE_");

var pll = cpp_expression("#define _US");

func szof(x: dynamic)
{
  return cpp_expression("#define _USE_");
}

var vi = cpp_expression("#define _US");

var vll = cpp_expression("#define _U");

var vvi = cpp_expression("#define _USE_MATH_D");

var vvll = cpp_expression("#define _USE_MATH_");

var INF = (1000000000 + 1e8);

var LINF = 2000000000000000000;

func print(a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      write(a[i], cpp_char(" "));
      i += 1;
    }
  }
  write(en);
}

func print(a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      {
        var j = 0;
        while ((j < a[i].size()))
        {
          write(a[i][j], cpp_char(" "));
          j += 1;
        }
      }
      write(en);
      i += 1;
    }
  }
}

func input(a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      read(a[i]);
      i += 1;
    }
  }
}

func input(a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      {
        var j = 0;
        while ((j < a[i].size()))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func gcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var x1: dynamic;
  var y1: dynamic;
  var g = gcd(b, (a % b), x1, y1);
  x = y1;
  y = (x1 - (((a / b)) * y1));
  return g;
}

func crt(a1: dynamic, a2: dynamic, n1: dynamic, n2: dynamic)
{
  var x1: dynamic;
  var y1: dynamic;
  var d = gcd(n1, n2, x1, y1);
  if (((((a1 - a2)) % d) != 0))
  {
    return -1;
  }
  var k1 = ((((a2 - a1)) / d) * x1);
  var t = (a1 + (n1 * ((k1 % n2))));
  while ((t < 0))
  {
    t += ((n1 * n2) / d);
  }
  if ((t > (((n1 * n2)) / d)))
  {
    t %= (((n1 * n2)) / d);
  }
  return t;
}

var lccm: dynamic;

var pos1: dynamic;

var pos2: dynamic;

var N = 0;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var crts: dynamic;

func get(cnt: dynamic)
{
  var ans = cnt;
  {
    var i = 0;
    while ((i < N))
    {
      if (((pos1[i] == -1) || (pos2[i] == -1)))
      {
        i += 1;
        continue;
      } else
      {
        var fst = crts[i];
        if ((fst != -1))
        {
          if (((cnt - fst) > 0))
          {
            var t = (cnt - fst);
            ans -= ((((t + lccm) - 1)) / lccm);
          }
        }
      }
      i += 1;
    }
  }
  return ans;
}

func solve()
{
  read(n, m, k);
  input(a);
  input(b);
  N = (*max_element(all(a)));
  N = max(N, (*max_element(all(b))));
  N += 1;
  input(a);
  input(b);
  pos1.resize(N);
  pos2.resize(N);
  fill(all(pos1), -1);
  fill(all(pos2), -1);
  crts.resize(N);
  var x: dynamic;
  var y: dynamic;
  lccm = ((n * m) / gcd(n, m, x, y));
  {
    var i = 0;
    while ((i < n))
    {
      pos1[a[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      pos2[b[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (((pos1[i] == -1) || (pos2[i] == -1)))
      {
        i += 1;
        continue;
      } else
      {
        crts[i] = crt(pos1[i], pos2[i], n, m);
      }
      i += 1;
    }
  }
  var l = 0;
  var r = 1e18;
  while (((r - l) > 1))
  {
    var m = (((l + r)) / 2);
    if ((get(m) >= k))
    {
      r = m;
    } else
    {
      l = m;
    }
  }
  write((l + 1));
}

func main()
{
  srand(time(0));
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  freopen("input.txt", "r", stdin);
  var tst = 1;
  while (cpp_update(tst, "--"))
  {
    solve();
  }
}
