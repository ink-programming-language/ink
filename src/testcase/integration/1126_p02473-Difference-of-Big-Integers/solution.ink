// Translated from solution.cpp.

class BigInteger
{
  var val: dynamic;
  var neg: dynamic;
  var u: dynamic;
  func BigInteger(a: dynamic)
  {
      this->val = cpp_construct(a);
    }
  func BigInteger(a: dynamic)
  {
      val.clear();
      val.push_back(a);
    }
  func BigInteger(s: dynamic)
  {
      var tmp = 0;
      var l = cpp_cast(s.size());
      var sh = 0;
      if ((s[0] == cpp_char("-")))
      {
        neg = 1;
        sh = 1;
      }
      {
        var i = sh;
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
  func BigInteger()
  {
      val.push_back(0);
    }
  func supplies()
  {
      var l = val.size();
      {
        var i = (l - 1);
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
  func get(index: dynamic)
  {
      if ((size() > index))
      {
        return val[index];
      } else
      {
        return 0;
      }
    }
  func add(a: dynamic)
  {
      if ((this->neg != a.neg))
      {
        a.neg = (!a.neg);
        return sub(a);
      }
      var res: dynamic;
      var m = max(this->size(), a.size());
      res.val.resize(m);
      var carry = 0;
      {
        var i = 0;
        while ((i < m))
        {
          res.val[i] = ((((*this))[i] + a[i]) + carry);
          carry = (res.val[i] / u);
          res.val[i] %= u;
          i += 1;
        }
      }
      if ((carry > 0))
      {
        res.val.push_back(carry);
      }
      res.neg = this->neg;
      return res;
    }
  func sub(a: dynamic)
  {
      if ((this->neg != a.neg))
      {
        a.neg = (!a.neg);
        return add(a);
      }
      var m = max(this->size(), a.size());
      var res: dynamic;
      res.neg = this->neg;
      res.val.resize(m);
      var borrow = 0;
      if (unsinged_greater(a))
      {
        {
          var i = 0;
          while ((i < m))
          {
            res.val[i] = ((((*this))[i] - a[i]) - borrow);
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
          var i = 0;
          while ((i < m))
          {
            res.val[i] = ((a[i] - ((*this))[i]) - borrow);
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
  func size()
  {
      this->supplies();
      return this->val.size();
    }
  func unsinged_greater(a: dynamic)
  {
      if ((this->size() > a.size()))
      {
        return true;
      }
      if ((this->size() < a.size()))
      {
        return false;
      }
      var s = this->size();
      {
        var i = (s - 1);
        while ((i >= 0))
        {
          if ((((*this))[i] > a[i]))
          {
            return true;
          } else if ((((*this))[i] < a[i]))
          {
            return false;
          }
          i -= 1;
        }
      }
      return false;
    }
  func is_zero()
  {
      if (((this->size() == 1) && (val[0] == 0)))
      {
        return true;
      }
      return false;
    }
  func neg_zero()
  {
      if ((this->is_zero() && this->neg))
      {
        this->neg = 0;
      }
    }
  func signed_equal(a: dynamic)
  {
      if ((this->neg != a.neg))
      {
        return false;
      }
      if ((this->val.size() != a.size()))
      {
        return false;
      }
      {
        var i = 0;
        while ((i < this->val.size()))
        {
          if ((this->val[i] != a.val[i]))
          {
            return false;
          }
          i += 1;
        }
      }
      return true;
    }
  func signed_greater(a: dynamic)
  {
      this->neg_zero();
      a.neg_zero();
      if (this->neg)
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
  func to_string()
  {
      var res = "";
      var ss: dynamic;
      this->neg_zero();
      if (this->neg)
      {
        (ss << cpp_char("-"));
      }
      var l = val.size();
      {
        var i = (l - 1);
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
  func operator_index(a: dynamic)
  {
      return get(a);
    }
  func operator_add(a: dynamic)
  {
      return this->add(a);
    }
  func operator_subtract(a: dynamic)
  {
      return this->sub(a);
    }
  func operator_add_assign(a: dynamic)
  {
      var res = this->add(a);
      this->neg = res.neg;
      this->val = res.val;
    }
  func operator_subtract_assign(a: dynamic)
  {
      var res = this->sub(a);
      this->neg = res.neg;
      this->val = res.val;
    }
}

func put(a: dynamic)
{
  write(a, "\n");
}

func solve(a: dynamic, b: dynamic)
{
  b1 -= b2;
  return b1.to_string();
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  b1 -= b2;
  write(b1.to_string(), "\n");
  return 0;
}
