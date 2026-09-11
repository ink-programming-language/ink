// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var pi: dynamic = acos(-1.0);

var maxn: dynamic = (100000 + 10);

var mod: dynamic = (1e9 + 7);

func getchar() -> dynamic
{
  var BUFSIZE: dynamic = 100001;
  var buf: dynamic = cpp_array(BUFSIZE);
  var psta: dynamic = buf;
  var pend: dynamic = buf;
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

func read(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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

func read(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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

func read(x: dynamic) -> dynamic
{
  var in_cpp: dynamic = cpp_uninitialized();
  var Dec: dynamic = 0.1;
  var IsN: dynamic = false;
  var IsD: dynamic = false;
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

func read(x: dynamic) -> dynamic
{
  var in_cpp: dynamic = cpp_uninitialized();
  var Dec: dynamic = 0.1;
  var IsN: dynamic = false;
  var IsD: dynamic = false;
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

func read(x: dynamic) -> dynamic
{
  var tmp: dynamic = x;
  var in_cpp: dynamic = getchar();
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

var p: dynamic = cpp_array(17000);

var n: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

func f(x: dynamic) -> dynamic
{
  return ((((((A * x) * x) * x) + ((B * x) * x)) + (C * x)) + D);
}

func Count(i: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var t: dynamic = 0;
  var x: dynamic = 1;
  while (((x * i) <= n))
  {
    x *= i;
    t += 1;
  }
  var pre: dynamic = 0;
  while (t)
  {
    var no: dynamic = ((n / x) * f(i));
    ans += (t * ((no - pre)));
    pre += (no - pre);
    t -= 1;
    x /= i;
  }
  return ans;
}

func main() -> dynamic
{
  scanf("%d%d%d%d%d", (&n), (&A), (&B), (&C), (&D));
  var tot: dynamic = 0;
  {
    var i: dynamic = 2;
    while (((i * i) <= n))
    {
      var flag: dynamic = 0;
      {
        var j: dynamic = 2;
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
  var ans: dynamic = 0;
  ans += Count(2);
  ans += Count(3);
  {
    var i: dynamic = 5;
    var f: dynamic = 2;
    while ((i <= n))
    {
      ans += Count(i);
      i = (i + f);
      f = (6 - f);
    }
  }
  {
    var i: dynamic = 5;
    var f: dynamic = 2;
    while ((i <= n))
    {
      {
        var j: dynamic = 2;
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
