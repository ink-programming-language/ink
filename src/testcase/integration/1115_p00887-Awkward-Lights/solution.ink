// Translated from solution.cpp.

func toInt(s: dynamic)
{
  var v: dynamic;
  (sin >> v);
  return v;
}

func toString(x: dynamic)
{
  var sout: dynamic;
  (sout << x);
  return sout.str();
}

func sqr(x: dynamic)
{
  return (x * x);
}

func all(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func rall(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var pb = cpp_expression("#include");

var mp = cpp_expression("#include");

func each(i: dynamic, c: dynamic)
{
  cpp_macro("for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); ++i)");
}

func exist(s: dynamic, e: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func range(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <bi");
}

func clr(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func dump(x: dynamic)
{
  cpp_macro("cerr << #x << \" = \" << (x) << endl;");
}

func debug(x: dynamic)
{
  cpp_macro("cerr << #x << \" = \" << (x) << \" (L\" << __LINE__ << \")\" << \" \" << __FILE__ << endl;");
}

var eps = 1e-10;

var pi = acos(-1.0);

var INF = (1 << 62);

var inf = (1 << 29);

func check(a: dynamic)
{
  var n = a.size();
  var m = a[0].size();
  var r = 0;
  var c = 0;
  rep(i, (n - 1));
  {
    var index = -1;
    while (((c < m) && (index == -1)))
    {
      range(j, i, n);
      if (a[j][c])
      {
        index = j;
      }
      if ((index == -1))
      {
        c += 1;
      } else if ((i != index))
      {
        swap(a[i], a[index]);
      }
    }
    if ((index == -1))
    {
      break;
    }
    range(j, (i + 1), n);
    {
      var d = (a[j][c] / a[i][c]);
    }
    c += 1;
  }
  var rank = 0;
  var add = 0;
  r = 0;
  c = 0;
  while (((r < n) && (c < (m - 1))))
  {
    while (((c < (m - 1)) && (a[r][c] == 0)))
    {
      c += 1;
    }
    if ((c == (m - 1)))
    {
      break;
    }
    rank += 1;
    r += 1;
    while (((r < n) && (a[r][c] != 0)))
    {
      r += 1;
    }
    if ((r == n))
    {
      break;
    }
  }
  if ((r < n))
  {
    range(i, r, n);
  }
  if ((a[i][(m - 1)] != 0))
  {
    add = 1;
  }
  return (!add);
}

func main(argument_0: dynamic)
{
  var m: dynamic;
  var n: dynamic;
  var d: dynamic;
  while ((((cin >> m) >> n) >> d))
  {
    var num = (m * n);
    if ((num == 0))
    {
      break;
    }
    var state = cpp_construct(vec(0, (num + 1)), num);
    rep(i, n);
    write(check(state), "\n");
  }
  return 0;
}

func range(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic)
{
        a[j][k] += (a[i][k] * d);
        a[j][k] %= 2;
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var col = ((a * m) + b);
        var dist = (abs((i - a)) + abs((j - b)));
        if ((dist == d))
        {
          state[row][col] = 1;
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var row = ((i * m) + j);
      rep(a, n);
      state[row][row] = 1;
      read(state[row][num]);
    }
