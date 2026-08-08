// Translated from solution.cpp.

func r(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var int_cpp = dynamic;

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

class RollingHash
{
  var S: dynamic;
  var B: dynamic;
  var len: dynamic;
  var hash: dynamic;
  var p: dynamic;
  func RollingHash()
  {
    }
  func RollingHash(S: dynamic, B: dynamic = 1000000009)
  {
      this->S = cpp_construct(S);
      this->B = cpp_construct(B);
      this->len = cpp_construct(S.length());
      this->hash = cpp_construct((len + 1));
      this->p = cpp_construct((len + 1));
      hash[0] = 0;
      p[0] = 1;
      {
        var i = 0;
        while ((i < len))
        {
          hash[(i + 1)] = ((hash[i] * B) + S[i]);
          p[(i + 1)] = (p[i] * B);
          i += 1;
        }
      }
    }
  func find(l: dynamic, r: dynamic)
  {
      return (hash[r] - (hash[l] * p[(r - l)]));
    }
}

func main()
{
  var n: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  var t = s;
  var st: dynamic;
  reverse(t.begin(), t.end());
  if ((st.size() == 1))
  {
    write(2, "\n");
    return 0;
  }
  {
    var i = 2;
    while ((i <= n))
    {
      var idx = 0;
      var F = 0;
      var A = S.find(0, i);
      {
        var j = 0;
        while (true)
        {
          if (((j % 2) == 0))
          {
            if (((idx + i) > n))
            {
              var x = (n - idx);
              var has = S.find(0, x);
              if ((S.find(idx, n) != has))
              {
                F += 1;
                break;
              }
            } else
            {
              if ((S.find(idx, (idx + i)) != A))
              {
                F += 1;
                break;
              }
            }
          } else
          {
            if ((((n - idx) - i) < 0))
            {
              var has = S.find((i - ((n - idx))), i);
              if ((T.find(0, (n - idx)) != has))
              {
                F += 1;
                break;
              }
            } else
            {
              if ((T.find(((n - idx) - i), (n - idx)) != A))
              {
                F += 1;
                break;
              }
            }
          }
          idx += (i - 1);
          if ((idx > n))
          {
            break;
          }
          j += 1;
        }
      }
      if ((!F))
      {
        write(i, "\n");
        return 0;
      }
      i += 1;
    }
  }
}

func r(argument_0: dynamic, argument_1: dynamic)
{
    st.insert(s[i]);
  }
