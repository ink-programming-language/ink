// Translated from solution.cpp.

class BigInt
{
  func BigInt(initValue: dynamic = 0) -> dynamic
  {
      (*self) = initValue;
    }
  func BigInt(s: dynamic, sign: dynamic) -> dynamic
  {
      self->s = s;
      self->sign = sign;
    }
  func operator_assign(value: dynamic) -> dynamic
  {
      sign =  ((value < 0)) ? -1 : 1;
      var absValue: dynamic = cpp_uninitialized();
      absValue =  ((value < 0)) ? (-value) : value;
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
      return (*self);
    }
  func operator_assign(other: dynamic) -> dynamic
  {
      sign = other.sign;
      s = other.s;
      return (*self);
    }
  func operator_add(other: dynamic) -> dynamic
  {
      if ((sign == other.sign))
      {
        var res: dynamic = other;
        {
          var i: dynamic = 0;
          var carry: dynamic = 0;
          while (((i < s.size()) || carry))
          {
            if ((i == res.s.size()))
            {
              res.s.push_back(0);
            }
            res.s[i] += (carry + ( ((i < s.size())) ? s[i] : 0));
            carry =  ((res.s[i] >= base)) ? 1 : 0;
            if (carry)
            {
              res.s[i] -= base;
            }
            i += 1;
          }
        }
        return res;
      }
      return ((*self) - ((-other)));
    }
  func operator_subtract() -> dynamic
  {
      var res: dynamic = (*self);
      res.sign = (-sign);
      return res;
    }
  func operator_subtract(other: dynamic) -> dynamic
  {
      if ((sign == other.sign))
      {
        if ((self->abs() >= other.abs()))
        {
          var res: dynamic = (*self);
          {
            var i: dynamic = 0;
            var carry: dynamic = 0;
            while (((i < other.s.size()) || carry))
            {
              res.s[i] -= (carry + ( ((i < other.s.size())) ? other.s[i] : 0));
              carry =  ((res.s[i] < 0)) ? 1 : 0;
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
        return (-((other - (*self))));
      }
      return ((*self) + ((-other)));
    }
  func operator_multiply(other: dynamic) -> dynamic
  {
      var res: dynamic = cpp_uninitialized();
      res.sign = (sign * other.sign);
      var add: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < other.s.size()))
        {
          add.push_back(((*self) * other.s[i]));
          i += 1;
        }
      }
      var maxLevel: dynamic = (s.size() * other.s.size());
      var carry: dynamic = 0;
      {
        var i: dynamic = 0;
        while (((i < maxLevel) || carry))
        {
          {
            var j: dynamic = 0;
            while (((j <= i) && (j < add.size())))
            {
              var pos: dynamic = (i - j);
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
  func operator_multiply(other: dynamic) -> dynamic
  {
      var res: dynamic = (*self);
      var value: dynamic = other;
      if ((value < 0))
      {
        res.sign = (-sign);
        value = (-value);
      }
      var carry: dynamic = 0;
      {
        var i: dynamic = 0;
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
  func operator_add_assign(other: dynamic) -> dynamic
  {
      (*self) = ((*self) + other);
    }
  func operator_subtract_assign(other: dynamic) -> dynamic
  {
      (*self) = ((*self) - other);
    }
  func operator(other: dynamic) -> dynamic
  {
      (*self) = ((*self) * other);
    }
  func abs() -> dynamic
  {
      var res: dynamic = (*self);
      res.sign = 1;
      return res;
    }
  func trim() -> dynamic
  {
      var i: dynamic = (s.size() - 1);
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
  func compare(other: dynamic) -> dynamic
  {
      if ((sign != other.sign))
      {
        return  ((sign == 1)) ? 1 : -1;
      }
      if ((s.size() != other.s.size()))
      {
        return  ((s.size() > other.s.size())) ? sign : (-sign);
      }
      {
        var i: dynamic = (s.size() - 1);
        while ((i >= 0))
        {
          if ((s[i] != other.s[i]))
          {
            return  ((s[i] > other.s[i])) ? sign : (-sign);
          }
          i -= 1;
        }
      }
      return 0;
    }
  func operator_less(other: dynamic) -> dynamic
  {
      return (compare(other) == -1);
    }
  func operator_equal(other: dynamic) -> dynamic
  {
      return (compare(other) == 0);
    }
  func operator_greater(other: dynamic) -> dynamic
  {
      return (compare(other) == 1);
    }
  func operator_less_equal(other: dynamic) -> dynamic
  {
      return (compare(other) <= 0);
    }
  func operator_greater_equal(other: dynamic) -> dynamic
  {
      return (compare(other) >= 0);
    }
  func toString() -> dynamic
  {
      var res: dynamic = cpp_uninitialized();
      if ((sign == -1))
      {
        res.push_back(cpp_char("-"));
      }
      var buf: dynamic = cpp_array(40);
      sprintf(buf, "%d", s.back());
      res += buf;
      {
        var i: dynamic = (s.size() - 2);
        while ((i >= 0))
        {
          sprintf(buf, "%09d", s[i]);
          res += buf;
          i -= 1;
        }
      }
      return res;
    }
  var base: dynamic = cpp_uninitialized();
  var sign: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
}

func operator_shift_left(os: dynamic, b: dynamic) -> dynamic
{
  return (os << b.toString());
}

var n: dynamic = cpp_uninitialized();

func checkAnswer(x: dynamic, y: dynamic) -> dynamic
{
  var count: dynamic = 0;
  var a: dynamic = 76717313154795141;
  var b: dynamic = 106780775536689089;
  var res: dynamic = cpp_uninitialized();
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

func checkAnswerStdout(x: dynamic, y: dynamic) -> dynamic
{
  printf(cpp_expression("\"%\""), (" %" + "\n"), x, y);
  fflush(stdout);
  var res: dynamic = cpp_uninitialized();
  scanf("%d", (&res));
  return res;
}

var hardMinA: dynamic = 1;

var hardMinB: dynamic = 1;

func guessRange(lowA: dynamic, hiA: dynamic, lowB: dynamic, hiB: dynamic, answerFn: dynamic) -> dynamic
{
  lowA = max(hardMinA, lowA);
  lowB = max(hardMinB, lowB);
  if (((lowA > hiA) || (lowB > hiB)))
  {
    return false;
  }
  var midA: dynamic = (lowA + (((hiA - lowA)) / 2));
  var midB: dynamic = (lowB + (((hiB - lowB)) / 2));
  var res: dynamic = answerFn(midA, midB);
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

func guessRange(answerFn: dynamic) -> dynamic
{
  guessRange(1, n, 1, n, answerFn);
}

class Space
{
  var lowX: dynamic = cpp_uninitialized();
  var midX: dynamic = cpp_uninitialized();
  var hiX: dynamic = cpp_uninitialized();
  var lowY: dynamic = cpp_uninitialized();
  var midY: dynamic = cpp_uninitialized();
  var hiY: dynamic = cpp_uninitialized();
  func Space(x1: dynamic, x2: dynamic, y1: dynamic, y2: dynamic) -> dynamic
  {
      lowX = x1;
      midX = x2;
      hiX = x2;
      lowY = y1;
      midY = y2;
      hiY = y2;
    }
  func areaA() -> dynamic
  {
      return (BigInt(((midX - lowX) + 1)) * BigInt(((midY - lowY) + 1)));
    }
  func areaB() -> dynamic
  {
      return (BigInt(((midX - lowX) + 1)) * BigInt((hiY - midY)));
    }
  func areaC() -> dynamic
  {
      return (BigInt((hiX - midX)) * BigInt(((midY - lowY) + 1)));
    }
  func setLowX(newLowX: dynamic) -> dynamic
  {
      lowX = newLowX;
      if ((lowX > midX))
      {
        (*self) = Space(lowX, hiX, lowY, midY);
      }
    }
  func setLowY(newLowY: dynamic) -> dynamic
  {
      lowY = newLowY;
      if ((lowY > midY))
      {
        (*self) = Space(lowX, midX, lowY, hiY);
      }
    }
  func setMidX(newMidX: dynamic) -> dynamic
  {
      midX = newMidX;
      if ((midX < lowX))
      {
        (*self) = Space(lowX, hiX, lowY, midY);
      }
    }
  func setMidY(newMidY: dynamic) -> dynamic
  {
      midY = newMidY;
      if ((midY < lowY))
      {
        (*self) = Space(lowX, midX, lowY, hiY);
      }
    }
}

func getMiddle(a: dynamic, b: dynamic) -> dynamic
{
  return (a + (((b - a)) / 2));
}

func guessRange(answerFn: dynamic) -> dynamic
{
  var space: dynamic = cpp_construct(1, n, 1, n);
  while (true)
  {
    var a: dynamic = space.areaA();
    var b: dynamic = space.areaB();
    var c: dynamic = space.areaC();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
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
    var res: dynamic = answerFn(x, y);
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

func main() -> dynamic
{
  if ((scanf(cpp_expression("\"%\""), PRId64, (&n)) == 1))
  {
    guessRange(checkAnswerStdout);
  }
}
