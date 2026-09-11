// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
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

func GG() -> dynamic
{
  puts("NO");
  exit(0);
}

var N: dynamic = (2e5 + 10);

var M: dynamic = (2e6 + 10);

var inf: dynamic = 1e18;

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var cnt1: dynamic = cpp_uninitialized();

var cnt2: dynamic = cpp_uninitialized();

class node
{
  var t: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

var v1: dynamic = cpp_array(N);

var v2: dynamic = cpp_array(N);

var flag: dynamic = cpp_array(2);

func solve(a: dynamic, m: dynamic) -> dynamic
{
  sort((a + 1), ((a + m) + 1), __cpp_lambda_1);
  flag[0] = cpp_assign(flag[1], "=", 0);
  var l: dynamic = (-inf);
  var r: dynamic = inf;
  var Div1: dynamic = __cpp_lambda_2;
  var Div2: dynamic = __cpp_lambda_3;
  {
    var i: dynamic = (2);
    while ((i <= (m)))
    {
      var dt: dynamic = (a[i].t - a[(i - 1)].t);
      var db: dynamic = (a[i].b - a[(i - 1)].b);
      var dk: dynamic = (a[i].k - a[(i - 1)].k);
      var L: dynamic = ((-dt) - db);
      var R: dynamic = (dt - db);
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
  var ans: dynamic = cpp_construct((l + 1));
  ans[0] = 1;
  {
    var i: dynamic = (1);
    while ((i < (m)))
    {
      var t: dynamic = (((a[(i + 1)].b + (a[(i + 1)].k * l))) - ((a[i].b + (a[i].k * l))));
      if ((t > 0))
      {
        var j: dynamic = (a[i].t + 1);
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
        var j: dynamic = (a[i].t + 1);
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

func main() -> dynamic
{
  n = read();
  l = read();
  {
    var i: dynamic = (1);
    while ((i <= (n)))
    {
      var t: dynamic = read();
      var x: dynamic = read();
      var y: dynamic = read();
      v1[cpp_update(cnt1, "++")] = [(t % l), ((-t) / l), (x - y)];
      v2[cpp_update(cnt2, "++")] = [(t % l), ((-t) / l), (x + y)];
      i += 1;
    }
  }
  v1[cpp_update(cnt1, "++")] = [0, 0, 0];
  v1[cpp_update(cnt1, "++")] = [l, 1, 0];
  v2[cpp_update(cnt2, "++")] = [0, 0, 0];
  v2[cpp_update(cnt2, "++")] = [l, 1, 0];
  var ansx: dynamic = solve(v1, cnt1);
  var ansy: dynamic = solve(v2, cnt2);
  {
    var i: dynamic = (1);
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

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (a.t < b.t);
}

func __cpp_lambda_2(a: dynamic, b: dynamic) -> dynamic
{
  if ((a >= 0))
  {
    return (a / b);
  }
  return (((-(((-a) - 1))) / b) - 1);
}

func __cpp_lambda_3(a: dynamic, b: dynamic) -> dynamic
{
  if ((a <= 0))
  {
    return (a / b);
  }
  return ((((a - 1)) / b) + 1);
}
