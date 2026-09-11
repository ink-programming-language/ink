// Translated from solution.cpp.

func r(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var int_cpp: dynamic = dynamic;

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

class RollingHash
{
  var S: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var len: dynamic = cpp_uninitialized();
  var hash: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func RollingHash() -> dynamic
  {
    }
  func RollingHash(S: dynamic, B: dynamic = 1000000009) -> dynamic
  {
      self->S = cpp_construct(S);
      self->B = cpp_construct(B);
      self->len = cpp_construct(S.length());
      self->hash = cpp_construct((len + 1));
      self->p = cpp_construct((len + 1));
      hash[0] = 0;
      p[0] = 1;
      {
        var i: dynamic = 0;
        while ((i < len))
        {
          hash[(i + 1)] = ((hash[i] * B) + S[i]);
          p[(i + 1)] = (p[i] * B);
          i += 1;
        }
      }
    }
  func find(l: dynamic, r: dynamic) -> dynamic
  {
      return (hash[r] - (hash[l] * p[(r - l)]));
    }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var t: dynamic = s;
  var st: dynamic = cpp_uninitialized();
  reverse(t.begin(), t.end());
  if ((st.size() == 1))
  {
    write(2, "\n");
    return 0;
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      var idx: dynamic = 0;
      var F: dynamic = 0;
      var A: dynamic = S.find(0, i);
      {
        var j: dynamic = 0;
        while (true)
        {
          if (((j % 2) == 0))
          {
            if (((idx + i) > n))
            {
              var x: dynamic = (n - idx);
              var has: dynamic = S.find(0, x);
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
              var has: dynamic = S.find((i - ((n - idx))), i);
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

func r(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    st.insert(s[i]);
  }
