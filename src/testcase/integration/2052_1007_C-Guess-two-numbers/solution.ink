// Translated from solution.cpp.

class BigInt
{
  func BigInt(initValue: dynamic = 0)
  {
      (*this) = initValue;
    }
  func BigInt(s: dynamic, sign: dynamic)
  {
      this->s = s;
      this->sign = sign;
    }
  func operator_assign(value: dynamic)
  {
      sign = if ((value < 0)) -1 else 1;
      var absValue: dynamic;
      absValue = if ((value < 0)) (-value) else value;
      s.clear();
      if ((absValue == 0))
      {
        s.push_back(0);
      } else
      {
        while ((absValue != 0))
        {
          s.push_back((absValue % base));
          absValue /= base;
        }
      }
      return (*this);
    }
  func operator_assign(other: dynamic)
  {
      sign = other.sign;
      s = other.s;
      return (*this);
    }
  func operator_add(other: dynamic)
  {
      if ((sign == other.sign))
      {
        var res = other;
        {
          var i = 0;
          var carry = 0;
          while (((i < s.size()) || carry))
          {
            if ((i == res.s.size()))
            {
              res.s.push_back(0);
            }
            res.s[i] += (carry + (if ((i < s.size())) s[i] else 0));
            carry = if ((res.s[i] >= base)) 1 else 0;
            if (carry)
            {
              res.s[i] -= base;
            }
            i += 1;
          }
        }
        return res;
      }
      return ((*this) - ((-other)));
    }
  func operator_subtract()
  {
      var res = (*this);
      res.sign = (-sign);
      return res;
    }
  func operator_subtract(other: dynamic)
  {
      if ((sign == other.sign))
      {
        if ((this->abs() >= other.abs()))
        {
          var res = (*this);
          {
            var i = 0;
            var carry = 0;
            while (((i < other.s.size()) || carry))
            {
              res.s[i] -= (carry + (if ((i < other.s.size())) other.s[i] else 0));
              carry = if ((res.s[i] < 0)) 1 else 0;
              if (carry)
              {
                res.s[i] += base;
              }
              i += 1;
            }
          }
          res.trim();
          return res;
        }
        return (-((other - (*this))));
      }
      return ((*this) + ((-other)));
    }
  func operator_multiply(other: dynamic)
  {
      var res: dynamic;
      res.sign = (sign * other.sign);
      var add: dynamic;
      {
        var i = 0;
        while ((i < other.s.size()))
        {
          add.push_back(((*this) * other.s[i]));
          i += 1;
        }
      }
      var maxLevel = (s.size() * other.s.size());
      var carry = 0;
      {
        var i = 0;
        while (((i < maxLevel) || carry))
        {
          {
            var j = 0;
            while (((j <= i) && (j < add.size())))
            {
              var pos = (i - j);
              if ((add[j].s.size() > pos))
              {
                carry += add[j].s[pos];
              }
              j += 1;
            }
          }
          if ((res.s.size() <= i))
          {
            res.s.resize((i + 1));
          }
          res.s[i] = (carry % base);
          carry /= base;
          i += 1;
        }
      }
      res.trim();
      return res;
    }
  func operator_multiply(other: dynamic)
  {
      var res = (*this);
      var value = other;
      if ((value < 0))
      {
        res.sign = (-sign);
        value = (-value);
      }
      var carry = 0;
      {
        var i = 0;
        while (((i < res.s.size()) || carry))
        {
          if ((i < res.s.size()))
          {
            carry += (cpp_cast(res.s[i]) * value);
          } else
          {
            res.s.push_back(0);
          }
          res.s[i] = (carry % base);
          carry /= base;
          i += 1;
        }
      }
      res.trim();
      return res;
    }
  func operator_add_assign(other: dynamic)
  {
      (*this) = ((*this) + other);
    }
  func operator_subtract_assign(other: dynamic)
  {
      (*this) = ((*this) - other);
    }
  func operator(other: dynamic)
  {
      (*this) = ((*this) * other);
    }
  func abs()
  {
      var res = (*this);
      res.sign = 1;
      return res;
    }
  func trim()
  {
      var i = (s.size() - 1);
      while (((i > 0) && (s[i] == 0)))
      {
        i -= 1;
      }
      s.resize((i + 1));
      if (((s.size() == 1) && (s[0] == 0)))
      {
        sign = 1;
      }
    }
  func compare(other: dynamic)
  {
      if ((sign != other.sign))
      {
        return if ((sign == 1)) 1 else -1;
      }
      if ((s.size() != other.s.size()))
      {
        return if ((s.size() > other.s.size())) sign else (-sign);
      }
      {
        var i = (s.size() - 1);
        while ((i >= 0))
        {
          if ((s[i] != other.s[i]))
          {
            return if ((s[i] > other.s[i])) sign else (-sign);
          }
          i -= 1;
        }
      }
      return 0;
    }
  func operator_less(other: dynamic)
  {
      return (compare(other) == -1);
    }
  func operator_equal(other: dynamic)
  {
      return (compare(other) == 0);
    }
  func operator_greater(other: dynamic)
  {
      return (compare(other) == 1);
    }
  func operator_less_equal(other: dynamic)
  {
      return (compare(other) <= 0);
    }
  func operator_greater_equal(other: dynamic)
  {
      return (compare(other) >= 0);
    }
  func toString()
  {
      var res: dynamic;
      if ((sign == -1))
      {
        res.push_back(cpp_char("-"));
      }
      var buf = cpp_array(40);
      sprintf(buf, "%d", s.back());
      res += buf;
      {
        var i = (s.size() - 2);
        while ((i >= 0))
        {
          sprintf(buf, "%09d", s[i]);
          res += buf;
          i -= 1;
        }
      }
      return res;
    }
  var base: dynamic;
  var sign: dynamic;
  var s: dynamic;
}

func operator_shift_left(os: dynamic, b: dynamic)
{
  return (os << b.toString());
}

var n: dynamic;

func checkAnswer(x: dynamic, y: dynamic)
{
  var count = 0;
  var a = 76717313154795141;
  var b = 106780775536689089;
  var res: dynamic;
  if (((x > a) || (y > b)))
  {
    res = 3;
  } else if ((x < a))
  {
    res = 1;
  } else if ((y < b))
  {
    res = 2;
  } else
  {
    res = 0;
  }
  printf(("%d try x = %" + ", y = %" + ", answer %d\n"), cpp_update(count, "++"), x, y, res);
  return res;
}

func checkAnswerStdout(x: dynamic, y: dynamic)
{
  printf(cpp_expression("\"%\""), (" %" + "\n"), x, y);
  fflush(stdout);
  var res: dynamic;
  scanf("%d", (&res));
  return res;
}

var hardMinA = 1;

var hardMinB = 1;

func guessRange(lowA: dynamic, hiA: dynamic, lowB: dynamic, hiB: dynamic, answerFn: dynamic)
{
  lowA = max(hardMinA, lowA);
  lowB = max(hardMinB, lowB);
  if (((lowA > hiA) || (lowB > hiB)))
  {
    return false;
  }
  var midA = (lowA + (((hiA - lowA)) / 2));
  var midB = (lowB + (((hiB - lowB)) / 2));
  var res = answerFn(midA, midB);
  if ((res == 0))
  {
    return true;
  }
  if ((res == 1))
  {
    hardMinA = (midA + 1);
    return guessRange((midA + 1), hiA, lowB, hiB, answerFn);
  }
  if ((res == 2))
  {
    hardMinB = (midB + 1);
    return guessRange(lowA, hiA, (midB + 1), hiB, answerFn);
  }
  return (guessRange(lowA, (midA - 1), lowB, hiB, answerFn) || guessRange(midA, hiA, lowB, (midB - 1), answerFn));
}

func guessRange(answerFn: dynamic)
{
  guessRange(1, n, 1, n, answerFn);
}

class Space
{
  var lowX: dynamic;
  var midX: dynamic;
  var hiX: dynamic;
  var lowY: dynamic;
  var midY: dynamic;
  var hiY: dynamic;
  func Space(x1: dynamic, x2: dynamic, y1: dynamic, y2: dynamic)
  {
      lowX = x1;
      midX = x2;
      hiX = x2;
      lowY = y1;
      midY = y2;
      hiY = y2;
    }
  func areaA()
  {
      return (BigInt(((midX - lowX) + 1)) * BigInt(((midY - lowY) + 1)));
    }
  func areaB()
  {
      return (BigInt(((midX - lowX) + 1)) * BigInt((hiY - midY)));
    }
  func areaC()
  {
      return (BigInt((hiX - midX)) * BigInt(((midY - lowY) + 1)));
    }
  func setLowX(newLowX: dynamic)
  {
      lowX = newLowX;
      if ((lowX > midX))
      {
        (*this) = Space(lowX, hiX, lowY, midY);
      }
    }
  func setLowY(newLowY: dynamic)
  {
      lowY = newLowY;
      if ((lowY > midY))
      {
        (*this) = Space(lowX, midX, lowY, hiY);
      }
    }
  func setMidX(newMidX: dynamic)
  {
      midX = newMidX;
      if ((midX < lowX))
      {
        (*this) = Space(lowX, hiX, lowY, midY);
      }
    }
  func setMidY(newMidY: dynamic)
  {
      midY = newMidY;
      if ((midY < lowY))
      {
        (*this) = Space(lowX, midX, lowY, hiY);
      }
    }
}

func getMiddle(a: dynamic, b: dynamic)
{
  return (a + (((b - a)) / 2));
}

func guessRange(answerFn: dynamic)
{
  var space = cpp_construct(1, n, 1, n);
  while (true)
  {
    var a = space.areaA();
    var b = space.areaB();
    var c = space.areaC();
    var x: dynamic;
    var y: dynamic;
    if ((b >= (a + c)))
    {
      x = getMiddle(space.lowX, space.midX);
      y = space.midY;
    } else if ((c > (a + b)))
    {
      x = space.midX;
      y = getMiddle(space.lowY, space.midY);
    } else
    {
      x = getMiddle(space.lowX, space.midX);
      y = getMiddle(space.lowY, space.midY);
    }
    var res = answerFn(x, y);
    if ((res == 0))
    {
      break;
    }
    if ((res == 1))
    {
      space.setLowX((x + 1));
    } else if ((res == 2))
    {
      space.setLowY((y + 1));
    } else if ((res == 3))
    {
      space.setMidX((x - 1));
      space.setMidY((y - 1));
    }
  }
}

func main()
{
  if ((scanf(cpp_expression("\"%\""), PRId64, (&n)) == 1))
  {
    guessRange(checkAnswerStdout);
  }
}
