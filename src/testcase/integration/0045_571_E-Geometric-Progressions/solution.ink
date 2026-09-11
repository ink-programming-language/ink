// Translated from solution.cpp.

var buf: dynamic = cpp_array(((1 << 19)));

var p1: dynamic = (buf + ((1 << 19)));

var pend: dynamic = (buf + ((1 << 19)));

func nc() -> dynamic
{
  if ((p1 == pend))
  {
    p1 = buf;
    pend = (buf + fread(buf, 1, ((1 << 19)), stdin));
  }
  return (*cpp_update(p1, "++"));
}

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var s: dynamic = nc();
  {
    while ((!isdigit(s)))
    {
      if ((s == cpp_char("-")))
      {
        f = -1;
      }
      s = nc();
    }
  }
  {
    while (isdigit(s))
    {
      x = (((((x << 1)) + ((x << 3))) + s) - cpp_char("0"));
      s = nc();
    }
  }
  return (x * f);
}

func mabs(x: dynamic) -> dynamic
{
  return  ((x > 0)) ? x : (-x);
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

class IT
{
  var p: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  func IT(p: dynamic = 0, k: dynamic = 0) -> dynamic
  {
      self->p = cpp_construct(p);
      self->k = cpp_construct(k);
    }
}

class rec
{
  var p: dynamic = cpp_array(1001);
  var num: dynamic = cpp_uninitialized();
  func init(x: dynamic) -> dynamic
  {
      {
        var i: dynamic = 2;
        while (((i * i) <= x))
        {
          if (((x % i) == 0))
          {
            var c: dynamic = 0;
            while (((x % i) == 0))
            {
              x /= i;
              c += 1;
            }
            p[cpp_update(num, "++")] = IT(i, c);
          }
          i += 1;
        }
      }
      if ((x > 1))
      {
        p[cpp_update(num, "++")] = IT(x, 1);
      }
    }
  func query(x: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < num))
        {
          if ((p[i].p == x))
          {
            return p[i];
          }
          i += 1;
        }
      }
      return IT(x, 0);
    }
}

var A: dynamic = cpp_array(200);

var B: dynamic = cpp_array(200);

var n: dynamic = cpp_uninitialized();

var A1: dynamic = cpp_array(3507);

var B1: dynamic = cpp_array(3507);

var A2: dynamic = cpp_array(3507);

var B2: dynamic = cpp_array(3507);

var pri: dynamic = cpp_array(3507);

var num: dynamic = cpp_uninitialized();

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((!b))
  {
    x = 1;
    y = 0;
    return a;
  }
  var g: dynamic = exgcd(b, (a % b), y, x);
  y -= ((a / b) * x);
  return g;
}

func inter(A: dynamic, B: dynamic, C: dynamic, a: dynamic, b: dynamic, c: dynamic, x1: dynamic, x2: dynamic) -> dynamic
{
  while (a)
  {
    var t: dynamic = (A / a);
    A -= (t * a);
    B -= (t * b);
    C -= (t * c);
    swap(A, a);
    swap(B, b);
    swap(C, c);
  }
  if ((c % b))
  {
    puts("-1");
    exit(0);
  }
  x2 = ((-c) / b);
  if ((((C + (B * x2))) % A))
  {
    puts("-1");
    exit(0);
  }
  x1 = ((((-C) - (B * x2))) / A);
}

func UN(a1: dynamic, b1: dynamic, a2: dynamic, b2: dynamic) -> dynamic
{
  num = 0;
  {
    var i: dynamic = 0;
    while ((i < a1.num))
    {
      pri[cpp_update(num, "++")] = a1.p[i].p;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < a2.num))
    {
      pri[cpp_update(num, "++")] = a2.p[i].p;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < b1.num))
    {
      pri[cpp_update(num, "++")] = b1.p[i].p;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < b2.num))
    {
      pri[cpp_update(num, "++")] = b2.p[i].p;
      i += 1;
    }
  }
  sort(pri, (pri + num));
  num = (unique(pri, (pri + num)) - pri);
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      A1[i] = a1.query(pri[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      A2[i] = a2.query(pri[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      B1[i] = b1.query(pri[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      B2[i] = b2.query(pri[i]);
      i += 1;
    }
  }
  var A: dynamic = 0;
  var B: dynamic = 0;
  var C: dynamic = 0;
  var flg1: dynamic = 0;
  var x1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      var a: dynamic = B1[i].k;
      var b: dynamic = (-B2[i].k);
      var c: dynamic = (A1[i].k - A2[i].k);
      if (((a == 0) && (b == 0)))
      {
        if (c)
        {
          puts("-1");
          exit(0);
        }
        i += 1;
        continue;
      }
      var g: dynamic = gcd(a, gcd((-b), mabs(c)));
      a /= g;
      b /= g;
      c /= g;
      if ((!b))
      {
        if ((c % a))
        {
          puts("-1");
          exit(0);
        }
        if ((((-c) / a) < 0))
        {
          puts("-1");
          exit(0);
        }
      }
      if (((!A) && (!B)))
      {
        A = a;
        B = b;
        C = c;
        i += 1;
        continue;
      }
      if ((!B))
      {
        if (b)
        {
          inter(A, B, C, a, b, c, x1, x2);
          flg1 = 1;
          break;
        }
        if (((C / A) != (c / a)))
        {
          puts("-1");
          exit(0);
        }
        i += 1;
        continue;
      }
      if (((A * b) == (a * B)))
      {
        if (((c * A) == (C * a)))
        {
          i += 1;
          continue;
        }
        puts("-1");
        exit(0);
      }
      inter(A, B, C, a, b, c, x1, x2);
      flg1 = 1;
      break;
      i += 1;
    }
  }
  if (flg1)
  {
    {
      var i: dynamic = 0;
      while ((i < num))
      {
        var a: dynamic = B1[i].k;
        var b: dynamic = (-B2[i].k);
        var c: dynamic = (A1[i].k - A2[i].k);
        if ((((a * x1) + (b * x2)) + c))
        {
          puts("-1");
          exit(0);
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < num))
      {
        A1[i].k = (A1[i].k + (B1[i].k * x1));
        B1[i].k = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < num))
      {
        a1.p[i] = A1[i];
        b1.p[i] = B1[i];
        i += 1;
      }
    }
    a1.num = cpp_assign(b1.num, "=", num);
    return;
  }
  var g: dynamic = exgcd(A, B, x1, x2);
  if ((C % g))
  {
    puts("-1");
    exit(0);
  }
  x1 *= ((-C) / g);
  x2 *= ((-C) / g);
  var tx: dynamic = mabs(((-B) / g));
  var ty: dynamic = mabs((A / g));
  if (((C > 0) || (!ty)))
  {
    x1 = ((((x1 % tx) + tx)) % tx);
    if (B)
    {
      x2 = ((-(((A * x1) + C))) / B);
    } else
    {
      x2 = 0;
    }
  } else
  {
    x2 = ((((x2 % ty) + ty)) % ty);
    if (A)
    {
      x1 = (((((-B) * x2) - C)) / A);
    } else
    {
      x1 = 0;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      A1[i].k = (A1[i].k + (B1[i].k * x1));
      B1[i].k = (tx * B1[i].k);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      a1.p[i] = A1[i];
      b1.p[i] = B1[i];
      i += 1;
    }
  }
  a1.num = cpp_assign(b1.num, "=", num);
}

var P: dynamic = (1e9 + 7);

func ksm(x: dynamic, y: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while (y)
  {
    if ((y & 1))
    {
      ans = ((ans * x) % P);
    }
    x = ((x * x) % P);
    y >>= 1;
  }
  return ans;
}

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    while ((i <= n))
    {
      a = read();
      b = read();
      A[i].init(a);
      B[i].init(b);
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      UN(A[1], B[1], A[i], B[i]);
      i += 1;
    }
  }
  var ans: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < A[1].num))
    {
      ans = (((ans * 1) * ksm(A[1].p[i].p, A[1].p[i].k)) % P);
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
