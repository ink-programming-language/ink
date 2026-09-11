// Translated from solution.cpp.

var fact: dynamic = cpp_construct(20);

var f_cnt: dynamic = cpp_construct(10);

func precalc() -> dynamic
{
  fact[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < 20))
    {
      fact[i] = (fact[(i - 1)] * i);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var st: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_construct(0);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var n: dynamic = s.length();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      f_cnt[(s[i] - cpp_char("0"))] += 1;
      i += 1;
    }
  }
  precalc();
  var mask: dynamic = (1 << n);
  {
    var i: dynamic = 0;
    while ((i < mask))
    {
      var cnt: dynamic = cpp_construct(10);
      var k: dynamic = cpp_construct(0);
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((((i & ((1 << j)))) != 0))
          {
            k += 1;
            cnt[(s[((n - j) - 1)] - cpp_char("0"))] += 1;
          }
          j += 1;
        }
      }
      var fz: dynamic = cpp_construct(1);
      var flag: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < 10))
        {
          if (((f_cnt[j] != 0) && (cnt[j] == 0)))
          {
            flag = 1;
          }
          j += 1;
        }
      }
      if ((flag || (st.find(cnt) != st.end())))
      {
        i += 1;
        continue;
      }
      st.insert(cnt);
      {
        var j: dynamic = 0;
        while ((j < 10))
        {
          fz *= fact[cnt[j]];
          j += 1;
        }
      }
      ans += (fact[k] / fz);
      if ((cnt[0] > 0))
      {
        k -= 1;
        cnt[0] -= 1;
        fz = 1;
        {
          var j: dynamic = 0;
          while ((j < 10))
          {
            fz *= fact[cnt[j]];
            j += 1;
          }
        }
        ans -= (fact[k] / fz);
      }
      i += 1;
    }
  }
  write(ans);
}
