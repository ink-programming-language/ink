// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if (a)
  {
    return gcd((b % a), a);
  } else
  {
    return b;
  }
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  var S = cpp_construct(s.size(), false);
  var rans = 0;
  {
    var i = 0;
    while (((i < s.size()) && (!S[i])))
    {
      var js = 1;
      var jt = 1;
      var in_s = cpp_construct(26, 0);
      var in_t = cpp_construct(26, 0);
      in_s[(s[i] - cpp_char("a"))] += 1;
      S[i] = true;
      {
        var j = (t.size() % s.size());
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
        var j = (s.size() % t.size());
        while (j)
        {
          in_t[(t[(((i + j)) % t.size())] - cpp_char("a"))] += 1;
          jt += 1;
          j = (((j + s.size())) % t.size());
        }
      }
      var ans = (js * jt);
      {
        var j = 0;
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
