// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#includ");

var MAXN: dynamic = cpp_expression("#inc");

var MAXSIZE: dynamic = cpp_expression("#include<s");

var DLEN: dynamic = cpp_expression("#");

class BigNum
{
  var a: dynamic = cpp_array((100005 / 2));
  var len: dynamic = cpp_uninitialized();
  func BigNum() -> dynamic
  {
      len = 1;
      memset(a, 0, cpp_sizeof((a)));
    }
}

func BigNum(b: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = b;
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

func BigNum(s: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var index: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
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
        var j: dynamic = k;
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

func BigNum(T: dynamic) -> dynamic
{
  cpp_base_construct(T.len);
  var i: dynamic = cpp_uninitialized();
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

func operator_assign(n: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
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
  return (*self);
}

func operator_shift_right(in_cpp: dynamic, b: dynamic) -> dynamic
{
  var ch: dynamic = cpp_array((MAXSIZE * 4));
  var i: dynamic = -1;
  (in_cpp >> ch);
  var l: dynamic = strlen(ch);
  var count: dynamic = 0;
  var sum: dynamic = 0;
  {
    i = (l - 1);
    while ((i >= 0))
    {
      sum = 0;
      var t: dynamic = 1;
      {
        var j: dynamic = 0;
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

func operator_shift_left(out: dynamic, b: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
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

func operator_add(T: dynamic) -> dynamic
{
  var t: dynamic = cpp_construct((*self));
  var i: dynamic = cpp_uninitialized();
  var big: dynamic = cpp_uninitialized();
  big =  ((T.len > len)) ? T.len : len;
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

func operator_subtract(T: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var big: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  if (((*self) > T))
  {
    t1 = (*self);
    t2 = T;
    flag = 0;
  } else
  {
    t1 = T;
    t2 = (*self);
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

func operator_multiply(T: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var up: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  var temp1: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < len))
    {
      i += 1;
    }
  }
  return ret;
}

func operator_divide(b: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  return ret;
}

func operator_greater(T: dynamic) -> dynamic
{
  var ln: dynamic = cpp_uninitialized();
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

func operator_greater(t: dynamic) -> dynamic
{
  return ((*self) > b);
}

func print() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
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

var A: dynamic = cpp_array(100005);

var B: dynamic = cpp_array(100005);

func main() -> dynamic
{
  scanf("%s", A);
  scanf("%s", B);
  if (((A[0] == cpp_char("-")) && (B[0] == cpp_char("-"))))
  {
    var a: dynamic = cpp_construct((A + 1));
    var b: dynamic = cpp_construct((B + 1));
    printf("-");
    a = (a + b);
    write(a);
  } else if (((A[0] == cpp_char("-")) && (B[0] != cpp_char("-"))))
  {
    var a: dynamic = cpp_construct((A + 1));
    var c: dynamic = cpp_uninitialized();
    c = (b - a);
    write(c);
  } else if (((A[0] != cpp_char("-")) && (B[0] == cpp_char("-"))))
  {
    var b: dynamic = cpp_construct((B + 1));
    var c: dynamic = cpp_uninitialized();
    c = (a - b);
    write(c);
  } else
  {
    a = (a + b);
    write(a);
  }
  write("\n");
}
