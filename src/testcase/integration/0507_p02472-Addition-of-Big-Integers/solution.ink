// Translated from solution.cpp.

var MAX = cpp_expression("#includ");

var MAXN = cpp_expression("#inc");

var MAXSIZE = cpp_expression("#include<s");

var DLEN = cpp_expression("#");

class BigNum
{
  var a: dynamic = cpp_array((100005 / 2));
  var len: dynamic;
  func BigNum()
  {
      len = 1;
      memset(a, 0, cpp_sizeof((a)));
    }
}

func BigNum(b: dynamic)
{
  var c: dynamic;
  var d = b;
  len = 0;
  memset(a, 0, cpp_sizeof((a)));
  while ((d > MAXN))
  {
    c = (d - (((d / ((MAXN + 1)))) * ((MAXN + 1))));
    d = (d / ((MAXN + 1)));
    a[cpp_update(len, "++")] = c;
  }
  a[cpp_update(len, "++")] = d;
}

func BigNum(s: dynamic)
{
  var t: dynamic;
  var k: dynamic;
  var index: dynamic;
  var l: dynamic;
  var i: dynamic;
  memset(a, 0, cpp_sizeof((a)));
  l = strlen(s);
  len = (l / DLEN);
  if ((l % DLEN))
  {
    len += 1;
  }
  index = 0;
  {
    i = (l - 1);
    while ((i >= 0))
    {
      t = 0;
      k = ((i - DLEN) + 1);
      if ((k < 0))
      {
        k = 0;
      }
      {
        var j = k;
        while ((j <= i))
        {
          t = (((t * 10) + s[j]) - cpp_char("0"));
          j += 1;
        }
      }
      a[cpp_update(index, "++")] = t;
      i -= DLEN;
    }
  }
}

func BigNum(T: dynamic)
{
  cpp_base_construct(T.len);
  var i: dynamic;
  memset(a, 0, cpp_sizeof((a)));
  {
    i = 0;
    while ((i < len))
    {
      a[i] = T.a[i];
      i += 1;
    }
  }
}

func operator_assign(n: dynamic)
{
  var i: dynamic;
  len = n.len;
  memset(a, 0, cpp_sizeof((a)));
  {
    i = 0;
    while ((i < len))
    {
      a[i] = n.a[i];
      i += 1;
    }
  }
  return (*this);
}

func operator_shift_right(in_cpp: dynamic, b: dynamic)
{
  var ch = cpp_array((MAXSIZE * 4));
  var i = -1;
  (in_cpp >> ch);
  var l = strlen(ch);
  var count = 0;
  var sum = 0;
  {
    i = (l - 1);
    while ((i >= 0))
    {
      sum = 0;
      var t = 1;
      {
        var j = 0;
        while (((j < 4) && (i >= 0)))
        {
          sum += (((ch[i] - cpp_char("0"))) * t);
          j += 1;
          i -= 1;
          t *= 10;
        }
      }
      b.a[count] = sum;
      count += 1;
    }
  }
  b.len = cpp_update(count, "++");
  return in_cpp;
}

func operator_shift_left(out: dynamic, b: dynamic)
{
  var i: dynamic;
  (out << b.a[(b.len - 1)]);
  {
    i = (b.len - 2);
    while ((i >= 0))
    {
      out.width(4);
      out.fill(cpp_char("0"));
      (out << b.a[i]);
      i -= 1;
    }
  }
  return out;
}

func operator_add(T: dynamic)
{
  var t = cpp_construct((*this));
  var i: dynamic;
  var big: dynamic;
  big = if ((T.len > len)) T.len else len;
  {
    i = 0;
    while ((i < big))
    {
      t.a[i] += T.a[i];
      if ((t.a[i] > MAXN))
      {
        t.a[(i + 1)] += 1;
        t.a[i] -= (MAXN + 1);
      }
      i += 1;
    }
  }
  if ((t.a[big] != 0))
  {
    t.len = (big + 1);
  } else
  {
    t.len = big;
  }
  return t;
}

func operator_subtract(T: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var big: dynamic;
  var flag: dynamic;
  var t1: dynamic;
  var t2: dynamic;
  if (((*this) > T))
  {
    t1 = (*this);
    t2 = T;
    flag = 0;
  } else
  {
    t1 = T;
    t2 = (*this);
    flag = 1;
  }
  big = t1.len;
  {
    i = 0;
    while ((i < big))
    {
      if ((t1.a[i] < t2.a[i]))
      {
        j = (i + 1);
        while ((t1.a[j] == 0))
        {
          j += 1;
        }
        t1.a[cpp_update(j, "--")] -= 1;
        while ((j > i))
        {
          t1.a[cpp_update(j, "--")] += MAXN;
        }
        t1.a[i] += ((MAXN + 1) - t2.a[i]);
      } else
      {
        t1.a[i] -= t2.a[i];
      }
      i += 1;
    }
  }
  t1.len = big;
  while (((t1.a[(t1.len - 1)] == 0) && (t1.len > 1)))
  {
    t1.len -= 1;
    big -= 1;
  }
  if (flag)
  {
    t1.a[(big - 1)] = (0 - t1.a[(big - 1)]);
  }
  return t1;
}

func operator_multiply(T: dynamic)
{
  var ret: dynamic;
  var i: dynamic;
  var j: dynamic;
  var up: dynamic;
  var temp: dynamic;
  var temp1: dynamic;
  {
    i = 0;
    while ((i < len))
    {
      i += 1;
    }
  }
  return ret;
}

func operator_divide(b: dynamic)
{
  var ret: dynamic;
  return ret;
}

func operator_greater(T: dynamic)
{
  var ln: dynamic;
  if ((len > T.len))
  {
    return true;
  } else if ((len == T.len))
  {
    ln = (len - 1);
    while (((a[ln] == T.a[ln]) && (ln >= 0)))
    {
      ln -= 1;
    }
    if (((ln >= 0) && (a[ln] > T.a[ln])))
    {
      return true;
    } else
    {
      return false;
    }
  } else
  {
    return false;
  }
}

func operator_greater(t: dynamic)
{
  return ((*this) > b);
}

func print()
{
  var i: dynamic;
  write(a[(len - 1)]);
  {
    i = (len - 2);
    while ((i >= 0))
    {
      cout.width(DLEN);
      cout.fill(cpp_char("0"));
      write(a[i]);
      i -= 1;
    }
  }
  write("\n");
}

var A = cpp_array(100005);

var B = cpp_array(100005);

func main()
{
  scanf("%s", A);
  scanf("%s", B);
  if (((A[0] == cpp_char("-")) && (B[0] == cpp_char("-"))))
  {
    var a = cpp_construct((A + 1));
    var b = cpp_construct((B + 1));
    printf("-");
    a = (a + b);
    write(a);
  } else if (((A[0] == cpp_char("-")) && (B[0] != cpp_char("-"))))
  {
    var a = cpp_construct((A + 1));
    var c: dynamic;
    c = (b - a);
    write(c);
  } else if (((A[0] != cpp_char("-")) && (B[0] == cpp_char("-"))))
  {
    var b = cpp_construct((B + 1));
    var c: dynamic;
    c = (a - b);
    write(c);
  } else
  {
    a = (a + b);
    write(a);
  }
  write("\n");
}
