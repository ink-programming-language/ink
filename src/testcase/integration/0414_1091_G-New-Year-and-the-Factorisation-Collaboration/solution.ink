// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

var BIGINTBITS = 32;

var BIGINTMASK = (((1 << BIGINTBITS)) - 1);

class BigInt
{
  var d: dynamic;
  func BigInt()
  {
    }
  func BigInt(x: dynamic)
  {
      while ((x != 0))
      {
        d.push_back((x & BIGINTMASK));
        x >>= BIGINTBITS;
      }
    }
  func val()
  {
      var ret = 0;
      {
        var i = ((cpp_cast((d).size())) - 1);
        while ((i >= 0))
        {
          ret = (((ret << BIGINTBITS)) | d[i]);
          i -= 1;
        }
      }
      return ret;
    }
}

func normalize(a: dynamic)
{
  while ((((cpp_cast((a.d).size())) > 0) && (a.d[((cpp_cast((a.d).size())) - 1)] == 0)))
  {
    a.d.pop_back();
  }
}

func cmp(a: dynamic, b: dynamic)
{
  if (((cpp_cast((a.d).size())) != (cpp_cast((b.d).size()))))
  {
    return if (((cpp_cast((a.d).size())) < (cpp_cast((b.d).size())))) -1 else +1;
  }
  {
    var i = ((cpp_cast((a.d).size())) - 1);
    while ((i >= 0))
    {
      if ((a.d[i] != b.d[i]))
      {
        return if ((a.d[i] < b.d[i])) -1 else +1;
      }
      i -= 1;
    }
  }
  return 0;
}

func operator_less(a: dynamic, b: dynamic)
{
  return (cmp(a, b) < 0);
}

func operator_less_equal(a: dynamic, b: dynamic)
{
  return (cmp(a, b) <= 0);
}

func operator_equal(a: dynamic, b: dynamic)
{
  return (cmp(a, b) == 0);
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  var carry = 0;
  {
    var i = 0;
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

func operator_add(a: dynamic, b: dynamic)
{
  var ret = a;
  ret += b;
  return ret;
}

func operator_subtract_assign(a: dynamic, b: dynamic)
{
  var carry = 0;
  {
    var i = 0;
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

func operator_subtract(a: dynamic, b: dynamic)
{
  var ret = a;
  ret -= b;
  return ret;
}

func operator_multiply(a: dynamic, b: dynamic)
{
  var ret: dynamic;
  {
    var j = 0;
    while ((j < (cpp_cast((b.d).size()))))
    {
      var carry = 0;
      {
        var i = 0;
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

func operator_multiply(a: dynamic, b: dynamic)
{
  assert(((0 <= b) && (b <= BIGINTMASK)));
  var carry = 0;
  var ret: dynamic;
  if ((b == 0))
  {
    return ret;
  }
  {
    var i = 0;
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

func operator_shift_left(a: dynamic, shift: dynamic)
{
  assert((shift < BIGINTBITS));
  var ret: dynamic;
  var carry = 0;
  {
    var i = 0;
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

func operator_shift_right(a: dynamic, shift: dynamic)
{
  assert((shift < BIGINTBITS));
  var ret: dynamic;
  var carry = 0;
  {
    var i = 0;
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

func dividewithremainder(a: dynamic, b: dynamic, q: dynamic, r: dynamic)
{
  assert(((1 <= b) && (b <= BIGINTMASK)));
  q.d.resize((cpp_cast((a.d).size())));
  var carry = 0;
  {
    var i = ((cpp_cast((a.d).size())) - 1);
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

func operator_divide(a: dynamic, b: dynamic)
{
  var q: dynamic;
  var r: dynamic;
  dividewithremainder(a, b, q, r);
  return q;
}

func operator_remainder(a: dynamic, b: dynamic)
{
  var q: dynamic;
  var r: dynamic;
  dividewithremainder(a, b, q, r);
  return r;
}

func dividewithremainder(a: dynamic, b: dynamic, q: dynamic, r: dynamic)
{
  if ((a < b))
  {
    q.d.clear();
    r = a;
    return;
  }
  if (((cpp_cast((b.d).size())) == 1))
  {
    var rr: dynamic;
    dividewithremainder(a, b.d[0], q, rr);
    r = BigInt(rr);
    return;
  }
  var shift = 0;
  while ((((((b.d[((cpp_cast((b.d).size())) - 1)] >> (((BIGINTBITS - shift) - 1)))) & 1)) == 0))
  {
    shift += 1;
  }
  var u = (a << shift);
  var v = (b << shift);
  q.d.resize((((cpp_cast((u.d).size())) - (cpp_cast((v.d).size()))) + 1));
  r.d.resize((cpp_cast((v.d).size())));
  {
    var i = 0;
    while ((i < (cpp_cast((v.d).size()))))
    {
      r.d[(((cpp_cast((v.d).size())) - i) - 1)] = u.d[(((cpp_cast((u.d).size())) - i) - 1)];
      i += 1;
    }
  }
  {
    var i = ((cpp_cast((q.d).size())) - 1);
    while ((i >= 0))
    {
      var num1 = if (((cpp_cast((v.d).size())) < (cpp_cast((r.d).size())))) r.d[(cpp_cast((v.d).size()))] else 0;
      var num2 = if ((((cpp_cast((v.d).size())) - 1) < (cpp_cast((r.d).size())))) r.d[((cpp_cast((v.d).size())) - 1)] else 0;
      var num = (((num1 << BIGINTBITS)) | num2);
      var den = v.d[((cpp_cast((v.d).size())) - 1)];
      var guess = min((num / den), cpp_cast(BIGINTMASK));
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

func operator_divide(a: dynamic, b: dynamic)
{
  var q: dynamic;
  var r: dynamic;
  dividewithremainder(a, b, q, r);
  return q;
}

func operator_remainder(a: dynamic, b: dynamic)
{
  var q: dynamic;
  var r: dynamic;
  dividewithremainder(a, b, q, r);
  return r;
}

func parse(s: dynamic, offset: dynamic, k: dynamic, xs: dynamic)
{
  if ((k == 0))
  {
    return BigInt(if (((0 <= offset) && (offset < (cpp_cast((s).size()))))) (s[offset] - cpp_char("0")) else 0);
  }
  return ((parse(s, offset, (k - 1), xs) * xs[k]) + parse(s, (offset + ((1 << ((k - 1))))), (k - 1), xs));
}

func parse(s: dynamic)
{
  var k = 0;
  while ((((1 << k)) < (cpp_cast((s).size()))))
  {
    k += 1;
  }
  var xs: dynamic;
  xs.push_back(BigInt(1));
  xs.push_back(BigInt(10));
  while ((k >= (cpp_cast((xs).size()))))
  {
    xs.push_back((xs.back() * xs.back()));
  }
  return parse(s, ((cpp_cast((s).size())) - ((1 << k))), k, xs);
}

func constsqr(a: dynamic)
{
  return (a * a);
}

func constpower(a: dynamic, n: dynamic)
{
  return if ((n == 0)) 1 else (constsqr(constpower(a, (n / 2))) * (if (((n % 2) == 0)) 1 else a));
}

var BIGDECIMALDIGITS = 9;

var BIGDECIMALBASE = constpower(10, BIGDECIMALDIGITS);

class BigDecimal
{
  var d: dynamic;
  func BigDecimal()
  {
    }
  func BigDecimal(x: dynamic)
  {
      while ((x > 0))
      {
        d.push_back((x % BIGDECIMALBASE));
        x /= BIGDECIMALBASE;
      }
    }
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  var carry = 0;
  {
    var i = 0;
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

func operator_add(a: dynamic, b: dynamic)
{
  var ret = a;
  ret += b;
  return ret;
}

func operator_multiply(a: dynamic, b: dynamic)
{
  var ret: dynamic;
  {
    var j = 0;
    while ((j < (cpp_cast((b.d).size()))))
    {
      var carry = 0;
      {
        var i = 0;
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

func format(a: dynamic, offset: dynamic, k: dynamic, xs: dynamic)
{
  if ((k == 0))
  {
    return BigDecimal(if (((0 <= offset) && (offset < (cpp_cast((a.d).size()))))) a.d[offset] else 0);
  }
  return ((format(a, offset, (k - 1), xs) * xs[k]) + format(a, (offset - ((1 << ((k - 1))))), (k - 1), xs));
}

func format(a: dynamic)
{
  var k = 0;
  while ((((1 << k)) < (cpp_cast((a.d).size()))))
  {
    k += 1;
  }
  var xs: dynamic;
  xs.push_back(BigDecimal(1));
  xs.push_back(BigDecimal((1 << BIGINTBITS)));
  while ((k >= (cpp_cast((xs).size()))))
  {
    xs.push_back((xs.back() * xs.back()));
  }
  var ans = format(a, (((1 << k)) - 1), k, xs);
  if (((cpp_cast((ans.d).size())) == 0))
  {
    return "0";
  }
  var ret = cpp_construct(((cpp_cast((ans.d).size())) * BIGDECIMALDIGITS), cpp_char("?"));
  {
    var i = (0);
    while ((i < ((cpp_cast((ans.d).size())))))
    {
      sprintf(((&ret[0]) + (i * BIGDECIMALDIGITS)), "%0*d", BIGDECIMALDIGITS, ans.d[(((cpp_cast((ans.d).size())) - i) - 1)]);
      i += 1;
    }
  }
  var nzero = 0;
  while (((nzero < (cpp_cast((ret).size()))) && (ret[nzero] == cpp_char("0"))))
  {
    nzero += 1;
  }
  ret = ret.substr(nzero);
  return ret;
}

func gcd(a: dynamic, b: dynamic)
{
  return if (((cpp_cast((b.d).size())) == 0)) a else gcd(b, (a % b));
}

func extractleadingbits(p: dynamic, q: dynamic, x: dynamic, y: dynamic)
{
  x = ((((cpp_cast(p.d[((cpp_cast((p.d).size())) - 1)])) << BIGINTBITS)) | p.d[((cpp_cast((p.d).size())) - 2)]);
  y = ((((cpp_cast((if (((cpp_cast((q.d).size())) == (cpp_cast((p.d).size())))) q.d[((cpp_cast((p.d).size())) - 1)] else 0))) << BIGINTBITS)) | q.d[((cpp_cast((p.d).size())) - 2)]);
  if (((cpp_cast((p.d).size())) == 2))
  {
    return;
  }
  var shift = 0;
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

func lehmergcd(p: dynamic, q: dynamic)
{
  var cmpres = cmp(p, q);
  if ((cmpres == 0))
  {
    return p;
  }
  if ((cmpres < 0))
  {
    swap(p, q);
  }
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  var num1: dynamic;
  var den1: dynamic;
  var w1: dynamic;
  var num2: dynamic;
  var den2: dynamic;
  var w2: dynamic;
  var e: dynamic;
  var f: dynamic;
  var xn: dynamic;
  var yn: dynamic;
  var t: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var w: dynamic;
  var needlongdiv: dynamic;
  var parity: dynamic;
  var nlong = 0;
  var nlehmer = 0;
  var clehmer: dynamic;
  var nit = 0;
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
        var i = 0;
        while ((i < (cpp_cast((p.d).size()))))
        {
          var cp = p.d[i];
          var cq = q.d[i];
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
      var r = (p % q);
      p = q;
      q = r;
      nlong += 1;
      nit += 1;
    }
  }
  x = ((((cpp_cast((if (((cpp_cast((p.d).size())) == 2)) p.d[1] else 0))) << BIGINTBITS)) | p.d[0]);
  y = ((((cpp_cast((if (((cpp_cast((q.d).size())) == 2)) q.d[1] else 0))) << BIGINTBITS)) | q.d[0]);
  while ((y != 0))
  {
    z = (x % y);
    x = y;
    y = z;
  }
  return BigInt(x);
}

func bitcnt(x: dynamic)
{
  if (((cpp_cast((x.d).size())) == 0))
  {
    return 0;
  }
  var r = 0;
  while ((x.d[((cpp_cast((x.d).size())) - 1)] >= ((1 << r))))
  {
    r += 1;
  }
  return (((((cpp_cast((x.d).size())) - 1)) * BIGINTBITS) + r);
}

func randbits(nbits: dynamic, rnd: dynamic)
{
  var ret: dynamic;
  var ndigs = ((((nbits + BIGINTBITS) - 1)) / BIGINTBITS);
  {
    var i = (0);
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

func pw(x: dynamic, n: dynamic, mod: dynamic)
{
  var ret = cpp_construct(1);
  {
    var i = (0);
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

func isprobableprime(n: dynamic, rnd: dynamic)
{
  if ((((cpp_cast((n.d).size())) == 1) && (((n.d[0] == 2) || (n.d[0] == 3)))))
  {
    return true;
  }
  if (((((cpp_cast((n.d).size())) == 0) || (((cpp_cast((n.d).size())) == 1) && (n.d[0] == 1))) || (((n.d[0] & 1)) == 0)))
  {
    return false;
  }
  var d = (n - 1);
  var r = 0;
  while ((d.d[0] == 0))
  {
    r += BIGINTBITS;
    d.d.erase(d.d.begin());
  }
  var rr = 0;
  while ((((d.d[0] & ((1 << rr)))) == 0))
  {
    rr += 1;
  }
  r += rr;
  d = (d >> rr);
  var alo = 2;
  var ahi = (n - 2);
  var ahibits = bitcnt(ahi);
  var xlo = 1;
  var xhi = (n - 1);
  {
    var k = (0);
    while ((k < (40)))
    {
      var a: dynamic;
      while (true)
      {
        a = randbits(ahibits, rnd);
        if (((alo <= a) && (a <= ahi)))
        {
          break;
        }
      }
      var x = pw(a, d, n);
      if (((x == xlo) || (x == xhi)))
      {
        k += 1;
        continue;
      }
      var ok = false;
      {
        var i = (0);
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

var local = false;

var ploc: dynamic;

var nloc: dynamic;

var locrnd: dynamic;

func egcd(a: dynamic, b: dynamic, x: dynamic, xneg: dynamic, y: dynamic, yneg: dynamic)
{
  if ((b == 0))
  {
    x = 1;
    xneg = false;
    y = 0;
    yneg = false;
    return a;
  }
  var g = egcd(b, (a % b), y, yneg, x, xneg);
  var z = (x * ((a / b)));
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

func invcrt(a1: dynamic, mod1: dynamic, a2: dynamic, mod2: dynamic)
{
  if ((a2 < a1))
  {
    swap(a1, a2);
    swap(mod1, mod2);
  }
  var c1neg: dynamic;
  var c2neg: dynamic;
  var c1: dynamic;
  var c2: dynamic;
  var g = egcd(mod1, mod2, c1, c1neg, c2, c2neg);
  assert(((((a2 - a1)) % g) == 0));
  var t = (((a2 - a1)) / g);
  var lcm = ((mod1 / g) * mod2);
  if (c1neg)
  {
    c1 = (mod2 - c1);
  }
  var x = (((a1 + (((c1 * t) % ((mod2 / g))) * mod1))) % lcm);
  return make_pair(x, lcm);
}

func invcrt(a: dynamic, mod: dynamic)
{
  var ret = make_pair(a[0], mod[0]);
  {
    var i = (1);
    while ((i < ((cpp_cast((a).size())))))
    {
      ret = invcrt(ret.first, ret.second, a[i], mod[i]);
      i += 1;
    }
  }
  return ret;
}

func query(x: dynamic)
{
  if ((!local))
  {
    printf("sqrt %s\n", format(x).c_str());
    fflush(stdout);
    var s: dynamic;
    read(s);
    assert((s != "-1"));
    return parse(s);
  } else
  {
    var a: dynamic;
    {
      var i = (0);
      while ((i < ((cpp_cast((ploc).size())))))
      {
        var cx = (x % ploc[i]);
        var cy = pw(cx, (((ploc[i] + 1)) / 4), ploc[i]);
        if (((locrnd() % 2) == 1))
        {
          cy = (ploc[i] - cy);
        }
        var A = ((cy * cy) % ploc[i]);
        var B = cx;
        assert((((cy * cy) % ploc[i]) == cx));
        a.push_back(cy);
        i += 1;
      }
    }
    var ret = invcrt(a, ploc).first;
    assert((((ret * ret) % nloc) == x));
    return ret;
  }
}

var ans: dynamic;

func solve(s: dynamic)
{
  var rnd = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));
  var n = parse(s);
  ans.clear();
  ans.push_back(n);
  while (true)
  {
    var x: dynamic;
    while (true)
    {
      x.d.clear();
      {
        var i = (0);
        while ((i < (((cpp_cast((n.d).size())) - 1))))
        {
          x.d.push_back(rnd());
          i += 1;
        }
      }
      var mxbit = 0;
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
    var y = ((x * x) % n);
    var z = query(y);
    if (((z == x) || (z == (n - x))))
    {
      continue;
    }
    var d = (((x + z)) % n);
    var nans: dynamic;
    {
      var i = (0);
      while ((i < ((cpp_cast((ans).size())))))
      {
        var g = lehmergcd(ans[i], d);
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
    var change = ((cpp_cast((nans).size())) != (cpp_cast((ans).size())));
    ans = nans;
    if (change)
    {
      var allprime = true;
      {
        var i = (0);
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

func run()
{
  var s: dynamic;
  read(s);
  solve(s);
  printf("! %d", (cpp_cast((ans).size())));
  {
    var i = (0);
    while ((i < ((cpp_cast((ans).size())))))
    {
      printf(" %s", format(ans[i]).c_str());
      i += 1;
    }
  }
  fflush(stdout);
}

func stressdivsmall()
{
  printf("\nstressdivsmall\n");
  {
    var rep = (0);
    while ((rep < (1000000)))
    {
      var ydig = ((rand() % 32) + 1);
      var y = 0;
      {
        var i = (0);
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
      var xdig = ((rand() % ((2 * ydig))) + 1);
      var x = 0;
      {
        var i = (0);
        while ((i < (xdig)))
        {
          x = (((x << 1)) + (rand() % 2));
          i += 1;
        }
      }
      var c = (a / b);
      var have = c.val();
      var want = (x / y);
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

func stressdivlarge()
{
  printf("\nstressdivlarge\n");
  {
    var rep = (0);
    while ((rep < (1000)))
    {
      var a: dynamic;
      a.d.resize(((((1000 + BIGINTBITS) - 1)) / BIGINTBITS));
      {
        var i = (0);
        while ((i < ((cpp_cast((a.d).size())))))
        {
          {
            var j = (0);
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
      var b: dynamic;
      b.d.resize(((((cpp_cast((a.d).size())) + 1)) / 2));
      {
        var i = (0);
        while ((i < ((cpp_cast((b.d).size())))))
        {
          {
            var j = (0);
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
      var c = (a / b);
      var d = (a - (b * c));
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

func stressparse()
{
  printf("\nverifying small\n");
  {
    var rep = (0);
    while ((rep < (100)))
    {
      var len = ((rand() % 18) + 1);
      var s = cpp_construct(len, cpp_char("?"));
      {
        var i = (0);
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
      var a = parse(s);
      var havenum = a.val();
      var wantnum: dynamic;
      sscanf(s.c_str(), "%llu", (&wantnum));
      if ((havenum != wantnum))
      {
        printf("err %s => havenum=%llu wantnum=%llu\n", s.c_str(), havenum, wantnum);
        return;
      }
      var havestr = format(a);
      var wantstr = s;
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
    var rep = (0);
    while ((rep < (100)))
    {
      var len = 10000;
      var s = cpp_construct(len, cpp_char("?"));
      {
        var i = (0);
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
      var a = parse(s);
      var have = format(a);
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

func stressgcd()
{
  printf("\nstressgcdsmall\n");
  printf("\nstressgcdlarge lehmer\n");
  {
    var rep = (0);
    while ((rep < (100000)))
    {
      var a: dynamic;
      a.d = vector(300, 0);
      {
        var i = (0);
        while ((i < ((cpp_cast((a.d).size())))))
        {
          {
            var j = (0);
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
      var b: dynamic;
      b.d = vector(300, 0);
      {
        var i = (0);
        while ((i < ((cpp_cast((b.d).size())))))
        {
          {
            var j = (0);
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
      var c = lehmergcd(a, b);
      var d = gcd(a, b);
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

func stressmillerrabin()
{
  var rnd = cpp_construct(123);
  {
    var rep = (0);
    while ((rep < (1000)))
    {
      var n = (rnd() % 1000);
      var nbits = (rnd() % 200);
      {
        var i = (0);
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

func stress()
{
  local = true;
  var targetbits = 1024;
  locrnd = mt19937(21312);
  {
    var rep = (0);
    while ((rep < (1000)))
    {
      nloc = BigInt(1);
      ploc.clear();
      var nprime = ((locrnd() % (((10 - 2) + 1))) + 2);
      {
        var i = (0);
        while ((i < (nprime)))
        {
          var mxpbits = (targetbits / ((i + 1)));
          var p: dynamic;
          while (true)
          {
            var x = randbits(((locrnd() % ((mxpbits - 2))) + 1), locrnd);
            p = ((4 * x) + 3);
            var have = false;
            {
              var j = (0);
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

func main()
{
  run();
  return 0;
}
