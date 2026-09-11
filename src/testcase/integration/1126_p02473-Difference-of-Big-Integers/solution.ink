// Translated from solution.cpp.

class BigInteger
{
  var val: dynamic = cpp_uninitialized();
  var neg: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  func BigInteger(a: dynamic) -> dynamic
  {
      self->val = cpp_construct(a);
    }
  func BigInteger(a: dynamic) -> dynamic
  {
      val.clear();
      val.push_back(a);
    }
  func BigInteger(s: dynamic) -> dynamic
  {
      var tmp: dynamic = 0;
      var l: dynamic = cpp_cast(s.size());
      var sh: dynamic = 0;
      if ((s[0] == cpp_char("-")))
      {
        neg = 1;
        sh = 1;
      }
      {
        var i: dynamic = sh;
        while ((i < l))
        {
          tmp *= 10;
          tmp += (cpp_cast(s[i]) - cpp_char("0"));
          if ((((((l - 1) - i)) % 9) == 0))
          {
            val.push_back(tmp);
            tmp = 0;
          }
          i += 1;
        }
      }
      reverse(val.begin(), val.end());
      supplies();
    }
  func BigInteger() -> dynamic
  {
      val.push_back(0);
    }
  func supplies() -> dynamic
  {
      var l: dynamic = val.size();
      {
        var i: dynamic = (l - 1);
        while ((i > 0))
        {
          if ((val[i] == 0))
          {
            val.pop_back();
          } else
          {
            break;
          }
          i -= 1;
        }
      }
    }
  func get(index: dynamic) -> dynamic
  {
      if ((size() > index))
      {
        return val[index];
      } else
      {
        return 0;
      }
    }
  func add(a: dynamic) -> dynamic
  {
      if ((self->neg != a.neg))
      {
        a.neg = (!a.neg);
        return sub(a);
      }
      var res: dynamic = cpp_uninitialized();
      var m: dynamic = max(self->size(), a.size());
      res.val.resize(m);
      var carry: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          res.val[i] = ((((*self))[i] + a[i]) + carry);
          carry = (res.val[i] / u);
          res.val[i] %= u;
          i += 1;
        }
      }
      if ((carry > 0))
      {
        res.val.push_back(carry);
      }
      res.neg = self->neg;
      return res;
    }
  func sub(a: dynamic) -> dynamic
  {
      if ((self->neg != a.neg))
      {
        a.neg = (!a.neg);
        return add(a);
      }
      var m: dynamic = max(self->size(), a.size());
      var res: dynamic = cpp_uninitialized();
      res.neg = self->neg;
      res.val.resize(m);
      var borrow: dynamic = 0;
      if (unsinged_greater(a))
      {
        {
          var i: dynamic = 0;
          while ((i < m))
          {
            res.val[i] = ((((*self))[i] - a[i]) - borrow);
            borrow = 0;
            if ((res.val[i] < 0))
            {
              borrow = ((((abs(res.val[i]) + u) - 1)) / u);
              res.val[i] += (borrow * u);
            }
            i += 1;
          }
        }
      } else
      {
        res.neg = (!res.neg);
        {
          var i: dynamic = 0;
          while ((i < m))
          {
            res.val[i] = ((a[i] - ((*self))[i]) - borrow);
            borrow = 0;
            if ((res.val[i] < 0))
            {
              borrow = ((((abs(res.val[i]) + u) - 1)) / u);
              res.val[i] += (borrow * u);
            }
            i += 1;
          }
        }
      }
      res.supplies();
      return res;
    }
  func size() -> dynamic
  {
      self->supplies();
      return self->val.size();
    }
  func unsinged_greater(a: dynamic) -> dynamic
  {
      if ((self->size() > a.size()))
      {
        return true;
      }
      if ((self->size() < a.size()))
      {
        return false;
      }
      var s: dynamic = self->size();
      {
        var i: dynamic = (s - 1);
        while ((i >= 0))
        {
          if ((((*self))[i] > a[i]))
          {
            return true;
          } else if ((((*self))[i] < a[i]))
          {
            return false;
          }
          i -= 1;
        }
      }
      return false;
    }
  func is_zero() -> dynamic
  {
      if (((self->size() == 1) && (val[0] == 0)))
      {
        return true;
      }
      return false;
    }
  func neg_zero() -> dynamic
  {
      if ((self->is_zero() && self->neg))
      {
        self->neg = 0;
      }
    }
  func signed_equal(a: dynamic) -> dynamic
  {
      if ((self->neg != a.neg))
      {
        return false;
      }
      if ((self->val.size() != a.size()))
      {
        return false;
      }
      {
        var i: dynamic = 0;
        while ((i < self->val.size()))
        {
          if ((self->val[i] != a.val[i]))
          {
            return false;
          }
          i += 1;
        }
      }
      return true;
    }
  func signed_greater(a: dynamic) -> dynamic
  {
      self->neg_zero();
      a.neg_zero();
      if (self->neg)
      {
        if (a.neg)
        {
          return (((!unsinged_greater(a))) || signed_equal(a));
        } else
        {
          return false;
        }
      } else
      {
        if (a.neg)
        {
          return true;
        } else
        {
          return unsinged_greater(a);
        }
      }
      return false;
    }
  func to_string() -> dynamic
  {
      var res: dynamic = "";
      var ss: dynamic = cpp_uninitialized();
      self->neg_zero();
      if (self->neg)
      {
        (ss << cpp_char("-"));
      }
      var l: dynamic = val.size();
      {
        var i: dynamic = (l - 1);
        while ((i >= 0))
        {
          if ((i == (l - 1)))
          {
            (ss << val[i]);
          } else
          {
            (((ss << setw(9)) << setfill(cpp_char("0"))) << val[i]);
          }
          i -= 1;
        }
      }
      return ss.str();
    }
  func operator_index(a: dynamic) -> dynamic
  {
      return get(a);
    }
  func operator_add(a: dynamic) -> dynamic
  {
      return self->add(a);
    }
  func operator_subtract(a: dynamic) -> dynamic
  {
      return self->sub(a);
    }
  func operator_add_assign(a: dynamic) -> dynamic
  {
      var res: dynamic = self->add(a);
      self->neg = res.neg;
      self->val = res.val;
    }
  func operator_subtract_assign(a: dynamic) -> dynamic
  {
      var res: dynamic = self->sub(a);
      self->neg = res.neg;
      self->val = res.val;
    }
}

func put(a: dynamic) -> dynamic
{
  write(a, "\n");
}

func solve(a: dynamic, b: dynamic) -> dynamic
{
  b1 -= b2;
  return b1.to_string();
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  b1 -= b2;
  write(b1.to_string(), "\n");
  return 0;
}
