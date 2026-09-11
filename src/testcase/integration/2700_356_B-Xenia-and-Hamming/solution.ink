// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if (a)
  {
    return gcd((b % a), a);
  } else
  {
    return b;
  }
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var S: dynamic = cpp_construct(s.size(), false);
  var rans: dynamic = 0;
  {
    var i: dynamic = 0;
    while (((i < s.size()) && (!S[i])))
    {
      var js: dynamic = 1;
      var jt: dynamic = 1;
      var in_s: dynamic = cpp_construct(26, 0);
      var in_t: dynamic = cpp_construct(26, 0);
      in_s[(s[i] - cpp_char("a"))] += 1;
      S[i] = true;
      {
        var j: dynamic = (t.size() % s.size());
        while (j)
        {
          in_s[(s[(((i + j)) % s.size())] - cpp_char("a"))] += 1;
          S[(((i + j)) % s.size())] = true;
          js += 1;
          j = (((j + t.size())) % s.size());
        }
      }
      in_t[(t[i] - cpp_char("a"))] += 1;
      {
        var j: dynamic = (s.size() % t.size());
        while (j)
        {
          in_t[(t[(((i + j)) % t.size())] - cpp_char("a"))] += 1;
          jt += 1;
          j = (((j + s.size())) % t.size());
        }
      }
      var ans: dynamic = (js * jt);
      {
        var j: dynamic = 0;
        while ((j < 26))
        {
          ans -= (in_s[j] * in_t[j]);
          j += 1;
        }
      }
      rans += ans;
      i += 1;
    }
  }
  write((rans * ((a / ((t.size() / gcd(s.size(), t.size())))))));
  return 0;
}
