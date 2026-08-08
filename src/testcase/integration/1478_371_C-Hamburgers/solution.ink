// Translated from solution.cpp.

func GCD(a: dynamic, b: dynamic)
{
  return if ((a)) GCD((b % a), a) else b;
}

func LCM(a: dynamic, b: dynamic)
{
  return ((a * b) / GCD(a, b));
}

func fastpow(b: dynamic, p: dynamic)
{
  if ((!p))
  {
    return 1;
  }
  var ret = fastpow(b, (p >> 1));
  ret *= ret;
  if ((p & 1))
  {
    ret *= b;
  }
  return ret;
}

var alpha = "abcdefghijklmnopqrstuvwxyz";

func divisor(number: dynamic)
{
  var i: dynamic;
  {
    i = 2;
    while ((i <= sqrt(number)))
    {
      if (((number % i) == 0))
      {
        return (number / i);
      }
      i += 1;
    }
  }
  return 1;
}

func myXOR(x: dynamic, y: dynamic)
{
  var res = 0;
  {
    var i = 31;
    while ((i >= 0))
    {
      var b1 = (x & ((1 << i)));
      var b2 = (y & ((1 << i)));
      var xoredBit = if (((b1 & b2))) 0 else ((b1 | b2));
      res <<= 1;
      res |= xoredBit;
      i -= 1;
    }
  }
  return res;
}

func printDivisors(n: dynamic, v: dynamic)
{
  {
    var i = 1;
    while ((i <= sqrt(n)))
    {
      if (((n % i) == 0))
      {
        if ((((n / i) == i) && (i > 1)))
        {
          v.push_back(i);
        } else
        {
          if ((i > 1))
          {
            v.push_back(i);
          }
          if (((n / i) > 1))
          {
            v.push_back((n / i));
          }
        }
      }
      i += 1;
    }
  }
}

func bin(vec: dynamic, val: dynamic)
{
  var l = 0;
  var r = (vec.size() - 1);
  var mid = (r / 2);
  while ((l <= r))
  {
    mid = (((l + r)) / 2);
    if (((vec[mid] < val) && (vec[(mid + 1)] > val)))
    {
      if ((vec[(mid + 1)] == val))
      {
        return (mid + 1);
      }
      return mid;
    } else if ((vec[mid] > val))
    {
      r = (mid - 1);
    } else if ((vec[mid] < val))
    {
      l = (mid + 1);
    } else if ((vec[mid] == val))
    {
      return mid;
    }
  }
  return -1;
}

func clear(v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      v[i] = 0;
      i += 1;
    }
  }
}

func comp(s1: dynamic, s2: dynamic)
{
  return ((s2 + s1) < (s1 + s2));
}

func split(s: dynamic, delim: dynamic)
{
  var result: dynamic;
  var item: dynamic;
  while (getline(ss, item, delim))
  {
    result.push_back(item);
  }
  return result;
}

func countWords(str: dynamic)
{
  var word: dynamic;
  var count = 0;
  while ((s >> word))
  {
    count += 1;
  }
  return count;
}

func IsLowerCharacter(c: dynamic)
{
  return ((c >= cpp_char("a")) && (c <= cpp_char("z")));
}

func main()
{
  var ham: dynamic;
  read(ham);
  var numOfB = 0;
  var numOfS = 0;
  var numOfC = 0;
  {
    var i = 0;
    while ((i < ham.length()))
    {
      if ((ham[i] == cpp_char("B")))
      {
        numOfB += 1;
      } else if ((ham[i] == cpp_char("S")))
      {
        numOfS += 1;
      } else if ((ham[i] == cpp_char("C")))
      {
        numOfC += 1;
      }
      i += 1;
    }
  }
  var nB: dynamic;
  var nS: dynamic;
  var nC: dynamic;
  var pB: dynamic;
  var pS: dynamic;
  var pC: dynamic;
  read(nB, nS, nC);
  read(pB, pS, pC);
  var r: dynamic;
  read(r);
  var num = 0;
  while (true)
  {
    if (((((nB - numOfB) < 0) || ((nS - numOfS) < 0)) || ((nC - numOfC) < 0)))
    {
      break;
    }
    if (((nB - numOfB) >= 0))
    {
      nB -= numOfB;
    } else
    {
      break;
    }
    if (((nS - numOfS) >= 0))
    {
      nS -= numOfS;
    } else
    {
      break;
    }
    if (((nC - numOfC) >= 0))
    {
      nC -= numOfC;
    } else
    {
      break;
    }
    num += 1;
  }
  while (((((((nB || nS) || nC)) && numOfC) && numOfB) && numOfS))
  {
    var price = 0;
    if ((nB < numOfB))
    {
      price += (((numOfB - nB)) * pB);
      nB = 0;
    } else
    {
      nB -= numOfB;
    }
    if ((nS < numOfS))
    {
      price += (((numOfS - nS)) * pS);
      nS = 0;
    } else
    {
      nS -= numOfS;
    }
    if ((nC < numOfC))
    {
      price += (((numOfC - nC)) * pC);
      nC = 0;
    } else
    {
      nC -= numOfC;
    }
    if ((price <= r))
    {
      r -= price;
      num += 1;
    } else
    {
      break;
    }
  }
  var priceOfoneHam = (((pB * numOfB) + (pS * numOfS)) + (pC * numOfC));
  num += (r / priceOfoneHam);
  write(num);
  return 0;
}
