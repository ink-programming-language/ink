// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var c = getchar();
  while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && ((c != cpp_char("-")))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return (x * f);
}

func GG()
{
  puts("NO");
  exit(0);
}

var N = (2e5 + 10);

var M = (2e6 + 10);

var inf = 1e18;

var n: dynamic;

var l: dynamic;

var cnt1: dynamic;

var cnt2: dynamic;

class node
{
  var t: dynamic;
  var k: dynamic;
  var b: dynamic;
}

var v1 = cpp_array(N);

var v2 = cpp_array(N);

var flag = cpp_array(2);

func solve(a: dynamic, m: dynamic)
{
  sort((a + 1), ((a + m) + 1), __cpp_lambda_1);
  flag[0] = cpp_assign(flag[1], "=", 0);
  var l = (-inf);
  var r = inf;
  var Div1 = __cpp_lambda_2;
  var Div2 = __cpp_lambda_3;
  {
    var i = (2);
    while ((i <= (m)))
    {
      var dt = (a[i].t - a[(i - 1)].t);
      var db = (a[i].b - a[(i - 1)].b);
      var dk = (a[i].k - a[(i - 1)].k);
      var L = ((-dt) - db);
      var R = (dt - db);
      if ((dk & 1))
      {
        flag[(((R & 1)) ^ 1)] = 1;
      } else if ((R & 1))
      {
        GG();
      }
      if ((!dk))
      {
        if ((L > R))
        {
          GG();
        }
      } else if ((dk < 0))
      {
        r = min(r, Div1((-L), (-dk)));
        l = max(l, Div2((-R), (-dk)));
      } else
      {
        r = min(r, Div1(R, dk));
        l = max(l, Div2(L, dk));
      }
      i += 1;
    }
  }
  if (flag[(l & 1)])
  {
    l += 1;
  }
  if ((flag[(l & 1)] || (l > r)))
  {
    GG();
  }
  var ans = cpp_construct((l + 1));
  ans[0] = 1;
  {
    var i = (1);
    while ((i < (m)))
    {
      var t = (((a[(i + 1)].b + (a[(i + 1)].k * l))) - ((a[i].b + (a[i].k * l))));
      if ((t > 0))
      {
        var j = (a[i].t + 1);
        while ((t > 0))
        {
          ans[j] = 1;
          t -= 1;
          j += 1;
        }
        while ((j <= a[(i + 1)].t))
        {
          ans[j] = (-ans[(j - 1)]);
          j += 1;
        }
      } else
      {
        var j = (a[i].t + 1);
        while ((t < 0))
        {
          ans[j] = -1;
          t += 1;
          j += 1;
        }
        while ((j <= a[(i + 1)].t))
        {
          ans[j] = (-ans[(j - 1)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func main()
{
  n = read();
  l = read();
  {
    var i = (1);
    while ((i <= (n)))
    {
      var t = read();
      var x = read();
      var y = read();
      v1[cpp_update(cnt1, "++")] = [(t % l), ((-t) / l), (x - y)];
      v2[cpp_update(cnt2, "++")] = [(t % l), ((-t) / l), (x + y)];
      i += 1;
    }
  }
  v1[cpp_update(cnt1, "++")] = [0, 0, 0];
  v1[cpp_update(cnt1, "++")] = [l, 1, 0];
  v2[cpp_update(cnt2, "++")] = [0, 0, 0];
  v2[cpp_update(cnt2, "++")] = [l, 1, 0];
  var ansx = solve(v1, cnt1);
  var ansy = solve(v2, cnt2);
  {
    var i = (1);
    while ((i <= (l)))
    {
      if (((ansx[i] == -1) && (ansy[i] == -1)))
      {
        putchar(cpp_char("L"));
      }
      if (((ansx[i] == 1) && (ansy[i] == 1)))
      {
        putchar(cpp_char("R"));
      }
      if (((ansx[i] == -1) && (ansy[i] == 1)))
      {
        putchar(cpp_char("U"));
      }
      if (((ansx[i] == 1) && (ansy[i] == -1)))
      {
        putchar(cpp_char("D"));
      }
      i += 1;
    }
  }
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (a.t < b.t);
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  if ((a >= 0))
  {
    return (a / b);
  }
  return (((-(((-a) - 1))) / b) - 1);
}

func __cpp_lambda_3(a: dynamic, b: dynamic)
{
  if ((a <= 0))
  {
    return (a / b);
  }
  return ((((a - 1)) / b) + 1);
}
