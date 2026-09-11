// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

var BIGINTBITS: dynamic = 32;

var BIGINTMASK: dynamic = (((1 << BIGINTBITS)) - 1);

class BigInt
{
  var d: dynamic = cpp_uninitialized();
  func BigInt() -> dynamic
  {
    }
  func BigInt(x: dynamic) -> dynamic
  {
      while ((x != 0))
      {
        d.push_back((x & BIGINTMASK));
        x >>= BIGINTBITS;
      }
    }
  func val() -> dynamic
  {
      var ret: dynamic = 0;
      {
        var i: dynamic = ((cpp_cast((d).size())) - 1);
        while ((i >= 0))
        {
          ret = (((ret << BIGINTBITS)) | d[i]);
          i -= 1;
        }
      }
      return ret;
    }
}

func normalize(a: dynamic) -> dynamic
{
  while ((((cpp_cast((a.d).size())) > 0) && (a.d[((cpp_cast((a.d).size())) - 1)] == 0)))
  {
    a.d.pop_back();
  }
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  if (((cpp_cast((a.d).size())) != (cpp_cast((b.d).size()))))
  {
    return  (((cpp_cast((a.d).size())) < (cpp_cast((b.d).size())))) ? -1 : +1;
  }
  {
    var i: dynamic = ((cpp_cast((a.d).size())) - 1);
    while ((i >= 0))
    {
      if ((a.d[i] != b.d[i]))
      {
        return  ((a.d[i] < b.d[i])) ? -1 : +1;
      }
      i -= 1;
    }
  }
  return 0;
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return (cmp(a, b) < 0);
}

func operator_less_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (cmp(a, b) <= 0);
}

func operator_equal(a: dynamic, b: dynamic) -> dynamic
{
  return (cmp(a, b) == 0);
}

func operator_add_assign(a: dynamic, b: dynamic) -> dynamic
{
  var carry: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < (cpp_cast((b.d).size()))) || (carry != 0)))
    {
      if ((i < (cpp_cast((a.d).size()))))
      {
        carry += a.d[i];
      } else
      {
        a.d.push_back(0);
      }
      if ((i < (cpp_cast((b.d).size()))))
      {
        carry += b.d[i];
      }
      a.d[i] = (carry & BIGINTMASK);
      carry >>= BIGINTBITS;
      i += 1;
    }
  }
  return a;
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = a;
  ret += b;
  return ret;
}

func operator_subtract_assign(a: dynamic, b: dynamic) -> dynamic
{
  var carry: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < (cpp_cast((b.d).size()))) || (carry != 0)))
    {
      if ((i < (cpp_cast((b.d).size()))))
      {
        carry += b.d[i];
      }
      assert((i < (cpp_cast((a.d).size()))));
      if ((carry <= a.d[i]))
      {
        a.d[i] -= carry;
        carry = 0;
      } else
      {
        a.d[i] += (((1 << BIGINTBITS)) - carry);
        carry = 1;
      }
      i += 1;
    }
  }
  normalize(a);
  return a;
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = a;
  ret -= b;
  return ret;
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  {
    var j: dynamic = 0;
    while ((j < (cpp_cast((b.d).size()))))
    {
      var carry: dynamic = 0;
      {
        var i: dynamic = 0;
        while (((i < (cpp_cast((a.d).size()))) || (carry != 0)))
        {
          if ((i < (cpp_cast((a.d).size()))))
          {
            carry += (cpp_cast(a.d[i]) * b.d[j]);
          }
          if (((i + j) < (cpp_cast((ret.d).size()))))
          {
            carry += ret.d[(i + j)];
          } else
          {
            ret.d.push_back(0);
          }
          ret.d[(i + j)] = (carry & BIGINTMASK);
          carry >>= BIGINTBITS;
          i += 1;
        }
      }
      j += 1;
    }
  }
  return ret;
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  assert(((0 <= b) && (b <= BIGINTMASK)));
  var carry: dynamic = 0;
  var ret: dynamic = cpp_uninitialized();
  if ((b == 0))
  {
    return ret;
  }
  {
    var i: dynamic = 0;
    while (((i < (cpp_cast((a.d).size()))) || (carry != 0)))
    {
      if ((i < (cpp_cast((a.d).size()))))
      {
        carry += (cpp_cast(a.d[i]) * b);
      }
      if ((i < (cpp_cast((ret.d).size()))))
      {
        carry += ret.d[i];
      } else
      {
        ret.d.push_back(0);
      }
      ret.d[i] = (carry & BIGINTMASK);
      carry >>= BIGINTBITS;
      i += 1;
    }
  }
  return ret;
}

func operator_shift_left(a: dynamic, shift: dynamic) -> dynamic
{
  assert((shift < BIGINTBITS));
  var ret: dynamic = cpp_uninitialized();
  var carry: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < (cpp_cast((a.d).size()))) || (carry != 0)))
    {
      if ((i < (cpp_cast((a.d).size()))))
      {
        carry |= ((cpp_cast(a.d[i])) << shift);
      }
      ret.d.push_back((carry & BIGINTMASK));
      carry >>= BIGINTBITS;
      i += 1;
    }
  }
  return ret;
}

func operator_shift_right(a: dynamic, shift: dynamic) -> dynamic
{
  assert((shift < BIGINTBITS));
  var ret: dynamic = cpp_uninitialized();
  var carry: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < (cpp_cast((a.d).size()))) || (carry != 0)))
    {
      if ((i < (cpp_cast((a.d).size()))))
      {
        carry |= ((cpp_cast(a.d[i])) << ((BIGINTBITS - shift)));
      }
      if ((i != 0))
      {
        ret.d.push_back((carry & BIGINTMASK));
      }
      carry >>= BIGINTBITS;
      i += 1;
    }
  }
  return ret;
}

func dividewithremainder(a: dynamic, b: dynamic, q: dynamic, r: dynamic) -> dynamic
{
  assert(((1 <= b) && (b <= BIGINTMASK)));
  q.d.resize((cpp_cast((a.d).size())));
  var carry: dynamic = 0;
  {
    var i: dynamic = ((cpp_cast((a.d).size())) - 1);
    while ((i >= 0))
    {
      carry <<= BIGINTBITS;
      carry += a.d[i];
      q.d[i] = (carry / b);
      carry -= (cpp_cast(q.d[i]) * b);
      i -= 1;
    }
  }
  normalize(q);
  r = carry;
}

func operator_divide(a: dynamic, b: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  dividewithremainder(a, b, q, r);
  return q;
}

func operator_remainder(a: dynamic, b: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  dividewithremainder(a, b, q, r);
  return r;
}

func dividewithremainder(a: dynamic, b: dynamic, q: dynamic, r: dynamic) -> dynamic
{
  if ((a < b))
  {
    q.d.clear();
    r = a;
    return;
  }
  if (((cpp_cast((b.d).size())) == 1))
  {
    var rr: dynamic = cpp_uninitialized();
    dividewithremainder(a, b.d[0], q, rr);
    r = BigInt(rr);
    return;
  }
  var shift: dynamic = 0;
  while ((((((b.d[((cpp_cast((b.d).size())) - 1)] >> (((BIGINTBITS - shift) - 1)))) & 1)) == 0))
  {
    shift += 1;
  }
  var u: dynamic = (a << shift);
  var v: dynamic = (b << shift);
  q.d.resize((((cpp_cast((u.d).size())) - (cpp_cast((v.d).size()))) + 1));
  r.d.resize((cpp_cast((v.d).size())));
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((v.d).size()))))
    {
      r.d[(((cpp_cast((v.d).size())) - i) - 1)] = u.d[(((cpp_cast((u.d).size())) - i) - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = ((cpp_cast((q.d).size())) - 1);
    while ((i >= 0))
    {
      var num1: dynamic =  (((cpp_cast((v.d).size())) < (cpp_cast((r.d).size())))) ? r.d[(cpp_cast((v.d).size()))] : 0;
      var num2: dynamic =  ((((cpp_cast((v.d).size())) - 1) < (cpp_cast((r.d).size())))) ? r.d[((cpp_cast((v.d).size())) - 1)] : 0;
      var num: dynamic = (((num1 << BIGINTBITS)) | num2);
      var den: dynamic = v.d[((cpp_cast((v.d).size())) - 1)];
      var guess: dynamic = min((num / den), cpp_cast(BIGINTMASK));
      while ((r < (v * guess)))
      {
        guess -= 1;
      }
      q.d[i] = guess;
      r -= (v * guess);
      if ((i != 0))
      {
        r.d.insert(r.d.begin(), u.d[(i - 1)]);
      }
      i -= 1;
    }
  }
  normalize(q);
  r = (r >> shift);
}

func operator_divide(a: dynamic, b: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  dividewithremainder(a, b, q, r);
  return q;
}

func operator_remainder(a: dynamic, b: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  dividewithremainder(a, b, q, r);
  return r;
}

func parse(s: dynamic, offset: dynamic, k: dynamic, xs: dynamic) -> dynamic
{
  if ((k == 0))
  {
    return BigInt( (((0 <= offset) && (offset < (cpp_cast((s).size()))))) ? (s[offset] - cpp_char("0")) : 0);
  }
  return ((parse(s, offset, (k - 1), xs) * xs[k]) + parse(s, (offset + ((1 << ((k - 1))))), (k - 1), xs));
}

func parse(s: dynamic) -> dynamic
{
  var k: dynamic = 0;
  while ((((1 << k)) < (cpp_cast((s).size()))))
  {
    k += 1;
  }
  var xs: dynamic = cpp_uninitialized();
  xs.push_back(BigInt(1));
  xs.push_back(BigInt(10));
  while ((k >= (cpp_cast((xs).size()))))
  {
    xs.push_back((xs.back() * xs.back()));
  }
  return parse(s, ((cpp_cast((s).size())) - ((1 << k))), k, xs);
}

func constsqr(a: dynamic) -> dynamic
{
  return (a * a);
}

func constpower(a: dynamic, n: dynamic) -> dynamic
{
  return  ((n == 0)) ? 1 : (constsqr(constpower(a, (n / 2))) * ( (((n % 2) == 0)) ? 1 : a));
}

var BIGDECIMALDIGITS: dynamic = 9;

var BIGDECIMALBASE: dynamic = constpower(10, BIGDECIMALDIGITS);

class BigDecimal
{
  var d: dynamic = cpp_uninitialized();
  func BigDecimal() -> dynamic
  {
    }
  func BigDecimal(x: dynamic) -> dynamic
  {
      while ((x > 0))
      {
        d.push_back((x % BIGDECIMALBASE));
        x /= BIGDECIMALBASE;
      }
    }
}

func operator_add_assign(a: dynamic, b: dynamic) -> dynamic
{
  var carry: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < (cpp_cast((b.d).size()))) || (carry != 0)))
    {
      if ((i < (cpp_cast((a.d).size()))))
      {
        carry += a.d[i];
      } else
      {
        a.d.push_back(0);
      }
      if ((i < (cpp_cast((b.d).size()))))
      {
        carry += b.d[i];
      }
      a.d[i] = (carry % BIGDECIMALBASE);
      carry /= BIGDECIMALBASE;
      i += 1;
    }
  }
  return a;
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = a;
  ret += b;
  return ret;
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  {
    var j: dynamic = 0;
    while ((j < (cpp_cast((b.d).size()))))
    {
      var carry: dynamic = 0;
      {
        var i: dynamic = 0;
        while (((i < (cpp_cast((a.d).size()))) || (carry != 0)))
        {
          if ((i < (cpp_cast((a.d).size()))))
          {
            carry += (cpp_cast(a.d[i]) * b.d[j]);
          }
          if (((i + j) < (cpp_cast((ret.d).size()))))
          {
            carry += ret.d[(i + j)];
          } else
          {
            ret.d.push_back(0);
          }
          ret.d[(i + j)] = (carry % BIGDECIMALBASE);
          carry /= BIGDECIMALBASE;
          i += 1;
        }
      }
      j += 1;
    }
  }
  return ret;
}

func format(a: dynamic, offset: dynamic, k: dynamic, xs: dynamic) -> dynamic
{
  if ((k == 0))
  {
    return BigDecimal( (((0 <= offset) && (offset < (cpp_cast((a.d).size()))))) ? a.d[offset] : 0);
  }
  return ((format(a, offset, (k - 1), xs) * xs[k]) + format(a, (offset - ((1 << ((k - 1))))), (k - 1), xs));
}

func format(a: dynamic) -> dynamic
{
  var k: dynamic = 0;
  while ((((1 << k)) < (cpp_cast((a.d).size()))))
  {
    k += 1;
  }
  var xs: dynamic = cpp_uninitialized();
  xs.push_back(BigDecimal(1));
  xs.push_back(BigDecimal((1 << BIGINTBITS)));
  while ((k >= (cpp_cast((xs).size()))))
  {
    xs.push_back((xs.back() * xs.back()));
  }
  var ans: dynamic = format(a, (((1 << k)) - 1), k, xs);
  if (((cpp_cast((ans.d).size())) == 0))
  {
    return "0";
  }
  var ret: dynamic = cpp_construct(((cpp_cast((ans.d).size())) * BIGDECIMALDIGITS), cpp_char("?"));
  {
    var i: dynamic = (0);
    while ((i < ((cpp_cast((ans.d).size())))))
    {
      sprintf(((&ret[0]) + (i * BIGDECIMALDIGITS)), "%0*d", BIGDECIMALDIGITS, ans.d[(((cpp_cast((ans.d).size())) - i) - 1)]);
      i += 1;
    }
  }
  var nzero: dynamic = 0;
  while (((nzero < (cpp_cast((ret).size()))) && (ret[nzero] == cpp_char("0"))))
  {
    nzero += 1;
  }
  ret = ret.substr(nzero);
  return ret;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (((cpp_cast((b.d).size())) == 0)) ? a : gcd(b, (a % b));
}

func extractleadingbits(p: dynamic, q: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  x = ((((cpp_cast(p.d[((cpp_cast((p.d).size())) - 1)])) << BIGINTBITS)) | p.d[((cpp_cast((p.d).size())) - 2)]);
  y = ((((cpp_cast(( (((cpp_cast((q.d).size())) == (cpp_cast((p.d).size())))) ? q.d[((cpp_cast((p.d).size())) - 1)] : 0))) << BIGINTBITS)) | q.d[((cpp_cast((p.d).size())) - 2)]);
  if (((cpp_cast((p.d).size())) == 2))
  {
    return;
  }
  var shift: dynamic = 0;
  while ((((((x >> ((((2 * BIGINTBITS) - shift) - 1)))) & 1)) == 0))
  {
    shift += 1;
  }
  if ((shift == 0))
  {
    return;
  }
  x = (((x << shift)) | ((p.d[((cpp_cast((p.d).size())) - 3)] >> ((BIGINTBITS - shift)))));
  y = (((y << shift)) | ((q.d[((cpp_cast((p.d).size())) - 3)] >> ((BIGINTBITS - shift)))));
}

func lehmergcd(p: dynamic, q: dynamic) -> dynamic
{
  var cmpres: dynamic = cmp(p, q);
  if ((cmpres == 0))
  {
    return p;
  }
  if ((cmpres < 0))
  {
    swap(p, q);
  }
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var num1: dynamic = cpp_uninitialized();
  var den1: dynamic = cpp_uninitialized();
  var w1: dynamic = cpp_uninitialized();
  var num2: dynamic = cpp_uninitialized();
  var den2: dynamic = cpp_uninitialized();
  var w2: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var xn: dynamic = cpp_uninitialized();
  var yn: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var needlongdiv: dynamic = cpp_uninitialized();
  var parity: dynamic = cpp_uninitialized();
  var nlong: dynamic = 0;
  var nlehmer: dynamic = 0;
  var clehmer: dynamic = cpp_uninitialized();
  var nit: dynamic = 0;
  while (true)
  {
    if (((cpp_cast((q.d).size())) == 0))
    {
      return p;
    } else if (((cpp_cast((p.d).size())) <= 2))
    {
      break;
    } else
    {
      needlongdiv = false;
    }
    if ((((cpp_cast((p.d).size())) - (cpp_cast((q.d).size()))) >= 2))
    {
      needlongdiv = true;
    }
    if ((!needlongdiv))
    {
      extractleadingbits(p, q, x, y);
      if (((y <= BIGINTMASK) || (x == y)))
      {
        needlongdiv = true;
      }
      if ((x == (((((cpp_cast(BIGINTMASK)) << BIGINTBITS)) | BIGINTMASK))))
      {
        x >>= 1;
        y >>= 1;
      }
    }
    if ((!needlongdiv))
    {
      num1 = x;
      den1 = (y + 1);
      num2 = (x + 1);
      den2 = y;
      w1 = (num1 / den1);
      w2 = (num2 / den2);
      if (((w1 != w2) || (w1 > BIGINTMASK)))
      {
        needlongdiv = true;
      } else
      {
        w = w1;
      }
    }
    if ((!needlongdiv))
    {
      a = 0;
      b = 1;
      c = 1;
      d = w;
      z = (x - (w * y));
      x = y;
      y = z;
      parity = 0;
      clehmer = 1;
      while (true)
      {
        if ((parity == 0))
        {
          if ((y == d))
          {
            break;
          }
          num1 = (x - a);
          den1 = (y + c);
          num2 = (x + b);
          den2 = (y - d);
        }
        if ((parity == 1))
        {
          if ((y == c))
          {
            break;
          }
          num1 = (x - b);
          den1 = (y + d);
          num2 = (x + a);
          den2 = (y - c);
        }
        w1 = (num1 / den1);
        w2 = (num2 / den2);
        if (((w1 != w2) || (w1 > BIGINTMASK)))
        {
          break;
        } else
        {
          w = w1;
        }
        e = (a + (w * c));
        f = (b + (w * d));
        z = (x - (w * y));
        if (((e > BIGINTMASK) || (f > BIGINTMASK)))
        {
          break;
        } else
        {
          a = c;
          c = e;
          b = d;
          d = f;
          x = y;
          y = z;
          parity = (1 - parity);
          clehmer += 1;
        }
      }
    }
    if (((!needlongdiv) && (b != 0)))
    {
      x = 0;
      y = 0;
      xn = 0;
      yn = 0;
      nlehmer += clehmer;
      nit += 1;
      while (((cpp_cast((q.d).size())) < (cpp_cast((p.d).size()))))
      {
        q.d.push_back(0);
      }
      {
        var i: dynamic = 0;
        while ((i < (cpp_cast((p.d).size()))))
        {
          var cp: dynamic = p.d[i];
          var cq: dynamic = q.d[i];
          if ((parity == 0))
          {
            x += (cq * b);
            xn += (cp * a);
            y += (cp * c);
            yn += (cq * d);
          } else
          {
            x += (cp * a);
            xn += (cq * b);
            y += (cq * d);
            yn += (cp * c);
          }
          t = min(x, xn);
          x -= t;
          xn -= t;
          t = min(y, yn);
          y -= t;
          yn -= t;
          if ((xn == 0))
          {
            p.d[i] = (x & BIGINTMASK);
            x >>= BIGINTBITS;
          } else if ((((xn & BIGINTMASK)) == 0))
          {
            p.d[i] = 0;
            xn >>= BIGINTBITS;
          } else
          {
            p.d[i] = ((BIGINTMASK - ((xn & BIGINTMASK))) + 1);
            xn >>= BIGINTBITS;
            xn += 1;
          }
          if ((yn == 0))
          {
            q.d[i] = (y & BIGINTMASK);
            y >>= BIGINTBITS;
          } else if ((((yn & BIGINTMASK)) == 0))
          {
            q.d[i] = 0;
            yn >>= BIGINTBITS;
          } else
          {
            q.d[i] = ((BIGINTMASK - ((yn & BIGINTMASK))) + 1);
            yn >>= BIGINTBITS;
            yn += 1;
          }
          i += 1;
        }
      }
      assert(((((x == 0) && (y == 0)) && (xn == 0)) && (yn == 0)));
      normalize(p);
      normalize(q);
    } else
    {
      var r: dynamic = (p % q);
      p = q;
      q = r;
      nlong += 1;
      nit += 1;
    }
  }
  x = ((((cpp_cast(( (((cpp_cast((p.d).size())) == 2)) ? p.d[1] : 0))) << BIGINTBITS)) | p.d[0]);
  y = ((((cpp_cast(( (((cpp_cast((q.d).size())) == 2)) ? q.d[1] : 0))) << BIGINTBITS)) | q.d[0]);
  while ((y != 0))
  {
    z = (x % y);
    x = y;
    y = z;
  }
  return BigInt(x);
}

func bitcnt(x: dynamic) -> dynamic
{
  if (((cpp_cast((x.d).size())) == 0))
  {
    return 0;
  }
  var r: dynamic = 0;
  while ((x.d[((cpp_cast((x.d).size())) - 1)] >= ((1 << r))))
  {
    r += 1;
  }
  return (((((cpp_cast((x.d).size())) - 1)) * BIGINTBITS) + r);
}

func randbits(nbits: dynamic, rnd: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  var ndigs: dynamic = ((((nbits + BIGINTBITS) - 1)) / BIGINTBITS);
  {
    var i: dynamic = (0);
    while ((i < ((ndigs - 1))))
    {
      ret.d.push_back(rnd());
      i += 1;
    }
  }
  ret.d.push_back((rnd() % ((1 << ((nbits - (((ndigs - 1)) * BIGINTBITS)))))));
  normalize(ret);
  return ret;
}

func pw(x: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  var ret: dynamic = cpp_construct(1);
  {
    var i: dynamic = (0);
    while ((i < (((cpp_cast((n.d).size())) * BIGINTBITS))))
    {
      if ((((n.d[(i / BIGINTBITS)] & ((1 << ((i % BIGINTBITS)))))) != 0))
      {
        ret = ((ret * x) % mod);
      }
      x = ((x * x) % mod);
      i += 1;
    }
  }
  return ret;
}

func isprobableprime(n: dynamic, rnd: dynamic) -> dynamic
{
  if ((((cpp_cast((n.d).size())) == 1) && (((n.d[0] == 2) || (n.d[0] == 3)))))
  {
    return true;
  }
  if (((((cpp_cast((n.d).size())) == 0) || (((cpp_cast((n.d).size())) == 1) && (n.d[0] == 1))) || (((n.d[0] & 1)) == 0)))
  {
    return false;
  }
  var d: dynamic = (n - 1);
  var r: dynamic = 0;
  while ((d.d[0] == 0))
  {
    r += BIGINTBITS;
    d.d.erase(d.d.begin());
  }
  var rr: dynamic = 0;
  while ((((d.d[0] & ((1 << rr)))) == 0))
  {
    rr += 1;
  }
  r += rr;
  d = (d >> rr);
  var alo: dynamic = 2;
  var ahi: dynamic = (n - 2);
  var ahibits: dynamic = bitcnt(ahi);
  var xlo: dynamic = 1;
  var xhi: dynamic = (n - 1);
  {
    var k: dynamic = (0);
    while ((k < (40)))
    {
      var a: dynamic = cpp_uninitialized();
      while (true)
      {
        a = randbits(ahibits, rnd);
        if (((alo <= a) && (a <= ahi)))
        {
          break;
        }
      }
      var x: dynamic = pw(a, d, n);
      if (((x == xlo) || (x == xhi)))
      {
        k += 1;
        continue;
      }
      var ok: dynamic = false;
      {
        var i: dynamic = (0);
        while ((i < ((r - 1))))
        {
          x = ((x * x) % n);
          if ((x == xhi))
          {
            ok = true;
            break;
          }
          i += 1;
        }
      }
      if (ok)
      {
        k += 1;
        continue;
      }
      return false;
      k += 1;
    }
  }
  return true;
}

var local: dynamic = false;

var ploc: dynamic = cpp_uninitialized();

var nloc: dynamic = cpp_uninitialized();

var locrnd: dynamic = cpp_uninitialized();

func egcd(a: dynamic, b: dynamic, x: dynamic, xneg: dynamic, y: dynamic, yneg: dynamic) -> dynamic
{
  if ((b == 0))
  {
    x = 1;
    xneg = false;
    y = 0;
    yneg = false;
    return a;
  }
  var g: dynamic = egcd(b, (a % b), y, yneg, x, xneg);
  var z: dynamic = (x * ((a / b)));
  if ((xneg != yneg))
  {
    y += z;
  } else if ((z <= y))
  {
    y -= z;
  } else
  {
    y = (z - y);
    yneg = (!yneg);
  }
  return g;
}

func invcrt(a1: dynamic, mod1: dynamic, a2: dynamic, mod2: dynamic) -> dynamic
{
  if ((a2 < a1))
  {
    swap(a1, a2);
    swap(mod1, mod2);
  }
  var c1neg: dynamic = cpp_uninitialized();
  var c2neg: dynamic = cpp_uninitialized();
  var c1: dynamic = cpp_uninitialized();
  var c2: dynamic = cpp_uninitialized();
  var g: dynamic = egcd(mod1, mod2, c1, c1neg, c2, c2neg);
  assert(((((a2 - a1)) % g) == 0));
  var t: dynamic = (((a2 - a1)) / g);
  var lcm: dynamic = ((mod1 / g) * mod2);
  if (c1neg)
  {
    c1 = (mod2 - c1);
  }
  var x: dynamic = (((a1 + (((c1 * t) % ((mod2 / g))) * mod1))) % lcm);
  return make_pair(x, lcm);
}

func invcrt(a: dynamic, mod: dynamic) -> dynamic
{
  var ret: dynamic = make_pair(a[0], mod[0]);
  {
    var i: dynamic = (1);
    while ((i < ((cpp_cast((a).size())))))
    {
      ret = invcrt(ret.first, ret.second, a[i], mod[i]);
      i += 1;
    }
  }
  return ret;
}

func query(x: dynamic) -> dynamic
{
  if ((!local))
  {
    printf("sqrt %s\n", format(x).c_str());
    fflush(stdout);
    var s: dynamic = cpp_uninitialized();
    read(s);
    assert((s != "-1"));
    return parse(s);
  } else
  {
    var a: dynamic = cpp_uninitialized();
    {
      var i: dynamic = (0);
      while ((i < ((cpp_cast((ploc).size())))))
      {
        var cx: dynamic = (x % ploc[i]);
        var cy: dynamic = pw(cx, (((ploc[i] + 1)) / 4), ploc[i]);
        if (((locrnd() % 2) == 1))
        {
          cy = (ploc[i] - cy);
        }
        var A: dynamic = ((cy * cy) % ploc[i]);
        var B: dynamic = cx;
        assert((((cy * cy) % ploc[i]) == cx));
        a.push_back(cy);
        i += 1;
      }
    }
    var ret: dynamic = invcrt(a, ploc).first;
    assert((((ret * ret) % nloc) == x));
    return ret;
  }
}

var ans: dynamic = cpp_uninitialized();

func solve(s: dynamic) -> dynamic
{
  var rnd: dynamic = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));
  var n: dynamic = parse(s);
  ans.clear();
  ans.push_back(n);
  while (true)
  {
    var x: dynamic = cpp_uninitialized();
    while (true)
    {
      x.d.clear();
      {
        var i: dynamic = (0);
        while ((i < (((cpp_cast((n.d).size())) - 1))))
        {
          x.d.push_back(rnd());
          i += 1;
        }
      }
      var mxbit: dynamic = 0;
      while ((n.d[((cpp_cast((n.d).size())) - 1)] >= ((2 << mxbit))))
      {
        mxbit += 1;
      }
      x.d.push_back((rnd() % ((2 << mxbit))));
      normalize(x);
      if ((x < n))
      {
        break;
      }
    }
    var y: dynamic = ((x * x) % n);
    var z: dynamic = query(y);
    if (((z == x) || (z == (n - x))))
    {
      continue;
    }
    var d: dynamic = (((x + z)) % n);
    var nans: dynamic = cpp_uninitialized();
    {
      var i: dynamic = (0);
      while ((i < ((cpp_cast((ans).size())))))
      {
        var g: dynamic = lehmergcd(ans[i], d);
        if (((g == 1) || (g == ans[i])))
        {
          nans.push_back(ans[i]);
        } else
        {
          nans.push_back(g);
          nans.push_back((ans[i] / g));
        }
        i += 1;
      }
    }
    var change: dynamic = ((cpp_cast((nans).size())) != (cpp_cast((ans).size())));
    ans = nans;
    if (change)
    {
      var allprime: dynamic = true;
      {
        var i: dynamic = (0);
        while ((i < ((cpp_cast((ans).size())))))
        {
          if ((!isprobableprime(ans[i], rnd)))
          {
            allprime = false;
            break;
          }
          i += 1;
        }
      }
      if (allprime)
      {
        break;
      }
    }
  }
  sort(ans.begin(), ans.end());
}

func run() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  solve(s);
  printf("! %d", (cpp_cast((ans).size())));
  {
    var i: dynamic = (0);
    while ((i < ((cpp_cast((ans).size())))))
    {
      printf(" %s", format(ans[i]).c_str());
      i += 1;
    }
  }
  fflush(stdout);
}

func stressdivsmall() -> dynamic
{
  printf("\nstressdivsmall\n");
  {
    var rep: dynamic = (0);
    while ((rep < (1000000)))
    {
      var ydig: dynamic = ((rand() % 32) + 1);
      var y: dynamic = 0;
      {
        var i: dynamic = (0);
        while ((i < (ydig)))
        {
          y = (((y << 1)) + (rand() % 2));
          i += 1;
        }
      }
      if ((y == 0))
      {
        rep += 1;
        continue;
      }
      var xdig: dynamic = ((rand() % ((2 * ydig))) + 1);
      var x: dynamic = 0;
      {
        var i: dynamic = (0);
        while ((i < (xdig)))
        {
          x = (((x << 1)) + (rand() % 2));
          i += 1;
        }
      }
      var c: dynamic = (a / b);
      var have: dynamic = c.val();
      var want: dynamic = (x / y);
      if ((have == want))
      {
        if (((rep % 1000) == 999))
        {
          printf(".");
        }
        rep += 1;
        continue;
      }
      printf("rep%d: %llu/%llu -> have=%llu want=%llu\n", rep, x, y, have, want);
      break;
      rep += 1;
    }
  }
}

func stressdivlarge() -> dynamic
{
  printf("\nstressdivlarge\n");
  {
    var rep: dynamic = (0);
    while ((rep < (1000)))
    {
      var a: dynamic = cpp_uninitialized();
      a.d.resize(((((1000 + BIGINTBITS) - 1)) / BIGINTBITS));
      {
        var i: dynamic = (0);
        while ((i < ((cpp_cast((a.d).size())))))
        {
          {
            var j: dynamic = (0);
            while ((j < (BIGINTBITS)))
            {
              a.d[i] |= (((rand() % 2)) << j);
              j += 1;
            }
          }
          i += 1;
        }
      }
      normalize(a);
      var b: dynamic = cpp_uninitialized();
      b.d.resize(((((cpp_cast((a.d).size())) + 1)) / 2));
      {
        var i: dynamic = (0);
        while ((i < ((cpp_cast((b.d).size())))))
        {
          {
            var j: dynamic = (0);
            while ((j < (BIGINTBITS)))
            {
              b.d[i] |= (((rand() % 2)) << j);
              j += 1;
            }
          }
          i += 1;
        }
      }
      normalize(b);
      if (((cpp_cast((b.d).size())) == 0))
      {
        rep += 1;
        continue;
      }
      var c: dynamic = (a / b);
      var d: dynamic = (a - (b * c));
      if ((d < b))
      {
        printf(".");
        rep += 1;
        continue;
      }
      printf("err\n");
      rep += 1;
    }
  }
}

func stressparse() -> dynamic
{
  printf("\nverifying small\n");
  {
    var rep: dynamic = (0);
    while ((rep < (100)))
    {
      var len: dynamic = ((rand() % 18) + 1);
      var s: dynamic = cpp_construct(len, cpp_char("?"));
      {
        var i: dynamic = (0);
        while ((i < (len)))
        {
          s[i] = (cpp_char("0") + (rand() % 10));
          i += 1;
        }
      }
      while ((((cpp_cast((s).size())) > 1) && (s[0] == cpp_char("0"))))
      {
        s = s.substr(1);
      }
      var a: dynamic = parse(s);
      var havenum: dynamic = a.val();
      var wantnum: dynamic = cpp_uninitialized();
      sscanf(s.c_str(), "%llu", (&wantnum));
      if ((havenum != wantnum))
      {
        printf("err %s => havenum=%llu wantnum=%llu\n", s.c_str(), havenum, wantnum);
        return;
      }
      var havestr: dynamic = format(a);
      var wantstr: dynamic = s;
      if ((havestr != wantstr))
      {
        printf("err %s => havestr=%s wantstr=%s\n", s.c_str(), havestr.c_str(), wantstr.c_str());
        return;
      }
      printf(".");
      rep += 1;
    }
  }
  printf("\ntesting large\n");
  {
    var rep: dynamic = (0);
    while ((rep < (100)))
    {
      var len: dynamic = 10000;
      var s: dynamic = cpp_construct(len, cpp_char("?"));
      {
        var i: dynamic = (0);
        while ((i < (len)))
        {
          s[i] = (cpp_char("0") + (rand() % 10));
          i += 1;
        }
      }
      while ((((cpp_cast((s).size())) > 1) && (s[0] == cpp_char("0"))))
      {
        s = s.substr(1);
      }
      var a: dynamic = parse(s);
      var have: dynamic = format(a);
      if ((have == s))
      {
        printf(".");
        rep += 1;
        continue;
      }
      printf("err\n");
      break;
      rep += 1;
    }
  }
}

func stressgcd() -> dynamic
{
  printf("\nstressgcdsmall\n");
  printf("\nstressgcdlarge lehmer\n");
  {
    var rep: dynamic = (0);
    while ((rep < (100000)))
    {
      var a: dynamic = cpp_uninitialized();
      a.d = vector(300, 0);
      {
        var i: dynamic = (0);
        while ((i < ((cpp_cast((a.d).size())))))
        {
          {
            var j: dynamic = (0);
            while ((j < (BIGINTBITS)))
            {
              a.d[i] |= (((rand() % 2)) << j);
              j += 1;
            }
          }
          i += 1;
        }
      }
      normalize(a);
      var b: dynamic = cpp_uninitialized();
      b.d = vector(300, 0);
      {
        var i: dynamic = (0);
        while ((i < ((cpp_cast((b.d).size())))))
        {
          {
            var j: dynamic = (0);
            while ((j < (BIGINTBITS)))
            {
              b.d[i] |= (((rand() % 2)) << j);
              j += 1;
            }
          }
          i += 1;
        }
      }
      normalize(b);
      var c: dynamic = lehmergcd(a, b);
      var d: dynamic = gcd(a, b);
      if ((format(c) != format(d)))
      {
        printf("err\n");
      }
      if (((rep % 1000) == 999))
      {
        printf(".");
      }
      rep += 1;
    }
  }
}

func stressmillerrabin() -> dynamic
{
  var rnd: dynamic = cpp_construct(123);
  {
    var rep: dynamic = (0);
    while ((rep < (1000)))
    {
      var n: dynamic = (rnd() % 1000);
      var nbits: dynamic = (rnd() % 200);
      {
        var i: dynamic = (0);
        while ((i < (nbits)))
        {
          n = (n << 1);
          i += 1;
        }
      }
      n = (n + 1);
      if (isprobableprime(n, rnd))
      {
        printf("%s is prime\n", format(n).c_str());
      }
      rep += 1;
    }
  }
}

func stress() -> dynamic
{
  local = true;
  var targetbits: dynamic = 1024;
  locrnd = mt19937(21312);
  {
    var rep: dynamic = (0);
    while ((rep < (1000)))
    {
      nloc = BigInt(1);
      ploc.clear();
      var nprime: dynamic = ((locrnd() % (((10 - 2) + 1))) + 2);
      {
        var i: dynamic = (0);
        while ((i < (nprime)))
        {
          var mxpbits: dynamic = (targetbits / ((i + 1)));
          var p: dynamic = cpp_uninitialized();
          while (true)
          {
            var x: dynamic = randbits(((locrnd() % ((mxpbits - 2))) + 1), locrnd);
            p = ((4 * x) + 3);
            var have: dynamic = false;
            {
              var j: dynamic = (0);
              while ((j < ((cpp_cast((ploc).size())))))
              {
                if ((ploc[j] == p))
                {
                  have = true;
                }
                j += 1;
              }
            }
            if (have)
            {
              continue;
            }
            if (isprobableprime(p, locrnd))
            {
              break;
            }
          }
          nloc = (nloc * p);
          ploc.push_back(p);
          i += 1;
        }
      }
      sort(ploc.begin(), ploc.end());
      printf("n=%s\n", format(nloc).c_str());
      solve(format(nloc));
      assert((ploc == ans));
      rep += 1;
    }
  }
}

func main() -> dynamic
{
  run();
  return 0;
}
