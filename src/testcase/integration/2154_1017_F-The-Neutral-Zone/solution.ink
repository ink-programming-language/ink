// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var INF = 0x3f3f3f3f3f3f3f3f;

var pi = acos(-1.0);

var maxn = (100000 + 10);

var mod = (1e9 + 7);

func getchar()
{
  var BUFSIZE = 100001;
  var buf = cpp_array(BUFSIZE);
  var psta = buf;
  var pend = buf;
  if ((psta >= pend))
  {
    psta = buf;
    pend = (buf + fread(buf, 1, BUFSIZE, stdin));
    if ((psta >= pend))
    {
      return -1;
    }
  }
  return (*cpp_update(psta, "++"));
}

func read(x: dynamic)
{
  x = 0;
  var f = 1;
  var ch = getchar();
  while (((((ch < cpp_char("0")) || (ch > cpp_char("9")))) && (~ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  if ((ch == -1))
  {
    return -1;
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  x *= f;
  return 1;
}

func read(x: dynamic)
{
  x = 0;
  var f = 1;
  var ch = getchar();
  while (((((ch < cpp_char("0")) || (ch > cpp_char("9")))) && (~ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  if ((ch == -1))
  {
    return -1;
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  x *= f;
  return 1;
}

func read(x: dynamic)
{
  var in_cpp: dynamic;
  var Dec = 0.1;
  var IsN = false;
  var IsD = false;
  in_cpp = getchar();
  if ((in_cpp == EOF))
  {
    return -1;
  }
  while ((((in_cpp != cpp_char("-")) && (in_cpp != cpp_char("."))) && (((in_cpp < cpp_char("0")) || (in_cpp > cpp_char("9"))))))
  {
    in_cpp = getchar();
  }
  if ((in_cpp == cpp_char("-")))
  {
    IsN = true;
    x = 0;
  } else if ((in_cpp == cpp_char(".")))
  {
    IsD = true;
    x = 0;
  } else
  {
    x = (in_cpp - cpp_char("0"));
  }
  if ((!IsD))
  {
    while (cpp_comma(cpp_assign(in_cpp, "=", getchar()), ((in_cpp >= cpp_char("0")) && (in_cpp <= cpp_char("9")))))
    {
      x *= 10;
      x += (in_cpp - cpp_char("0"));
    }
  }
  if ((in_cpp != cpp_char(".")))
  {
    if (IsN)
    {
      x = (-x);
    }
    return 1;
  } else
  {
    while (cpp_comma(cpp_assign(in_cpp, "=", getchar()), ((in_cpp >= cpp_char("0")) && (in_cpp <= cpp_char("9")))))
    {
      x += (Dec * ((in_cpp - cpp_char("0"))));
      Dec *= 0.1;
    }
  }
  if (IsN)
  {
    x = (-x);
  }
  return 1;
}

func read(x: dynamic)
{
  var in_cpp: dynamic;
  var Dec = 0.1;
  var IsN = false;
  var IsD = false;
  in_cpp = getchar();
  if ((in_cpp == EOF))
  {
    return -1;
  }
  while ((((in_cpp != cpp_char("-")) && (in_cpp != cpp_char("."))) && (((in_cpp < cpp_char("0")) || (in_cpp > cpp_char("9"))))))
  {
    in_cpp = getchar();
  }
  if ((in_cpp == cpp_char("-")))
  {
    IsN = true;
    x = 0;
  } else if ((in_cpp == cpp_char(".")))
  {
    IsD = true;
    x = 0;
  } else
  {
    x = (in_cpp - cpp_char("0"));
  }
  if ((!IsD))
  {
    while (cpp_comma(cpp_assign(in_cpp, "=", getchar()), ((in_cpp >= cpp_char("0")) && (in_cpp <= cpp_char("9")))))
    {
      x *= 10;
      x += (in_cpp - cpp_char("0"));
    }
  }
  if ((in_cpp != cpp_char(".")))
  {
    if (IsN)
    {
      x = (-x);
    }
    return 1;
  } else
  {
    while (cpp_comma(cpp_assign(in_cpp, "=", getchar()), ((in_cpp >= cpp_char("0")) && (in_cpp <= cpp_char("9")))))
    {
      x += (Dec * ((in_cpp - cpp_char("0"))));
      Dec *= 0.1;
    }
  }
  if (IsN)
  {
    x = (-x);
  }
  return 1;
}

func read(x: dynamic)
{
  var tmp = x;
  var in_cpp = getchar();
  while (((in_cpp <= cpp_char(" ")) && (in_cpp != EOF)))
  {
    in_cpp = getchar();
  }
  if ((in_cpp == -1))
  {
    return -1;
  }
  while ((in_cpp > cpp_char(" ")))
  {
    (*(cpp_update(tmp, "++"))) = in_cpp;
    in_cpp = getchar();
  }
  (*tmp) = cpp_char("\u{0}");
  return 1;
}

var p = cpp_array(17000);

var n: dynamic;

var A: dynamic;

var B: dynamic;

var C: dynamic;

var D: dynamic;

func f(x: dynamic)
{
  return ((((((A * x) * x) * x) + ((B * x) * x)) + (C * x)) + D);
}

func Count(i: dynamic)
{
  var ans = 0;
  var t = 0;
  var x = 1;
  while (((x * i) <= n))
  {
    x *= i;
    t += 1;
  }
  var pre = 0;
  while (t)
  {
    var no = ((n / x) * f(i));
    ans += (t * ((no - pre)));
    pre += (no - pre);
    t -= 1;
    x /= i;
  }
  return ans;
}

func main()
{
  scanf("%d%d%d%d%d", (&n), (&A), (&B), (&C), (&D));
  var tot = 0;
  {
    var i = 2;
    while (((i * i) <= n))
    {
      var flag = 0;
      {
        var j = 2;
        while (((j * j) <= i))
        {
          if (((i % j) == 0))
          {
            flag = 1;
            break;
          }
          j += 1;
        }
      }
      if ((!flag))
      {
        p[cpp_update(tot, "++")] = i;
      }
      i += 1;
    }
  }
  var ans = 0;
  ans += Count(2);
  ans += Count(3);
  {
    var i = 5;
    var f = 2;
    while ((i <= n))
    {
      ans += Count(i);
      i = (i + f);
      f = (6 - f);
    }
  }
  {
    var i = 5;
    var f = 2;
    while ((i <= n))
    {
      {
        var j = 2;
        while ((j < tot))
        {
          if ((((1 * i) * p[j]) > n))
          {
            break;
          }
          ans -= Count((i * p[j]));
          if (((i % p[j]) == 0))
          {
            break;
          }
          j += 1;
        }
      }
      i = (i + f);
      f = (6 - f);
    }
  }
  printf("%u", ans);
  return 0;
}
