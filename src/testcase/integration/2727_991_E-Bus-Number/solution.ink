// Translated from solution.cpp.

var fact = cpp_construct(20);

var f_cnt = cpp_construct(10);

func precalc()
{
  fact[0] = 1;
  {
    var i = 1;
    while ((i < 20))
    {
      fact[i] = (fact[(i - 1)] * i);
      i += 1;
    }
  }
}

func main()
{
  var st: dynamic;
  var ans = cpp_construct(0);
  var s: dynamic;
  read(s);
  var n = s.length();
  {
    var i = 0;
    while ((i < n))
    {
      f_cnt[(s[i] - cpp_char("0"))] += 1;
      i += 1;
    }
  }
  precalc();
  var mask = (1 << n);
  {
    var i = 0;
    while ((i < mask))
    {
      var cnt = cpp_construct(10);
      var k = cpp_construct(0);
      {
        var j = 0;
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
      var fz = cpp_construct(1);
      var flag = 0;
      {
        var j = 0;
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
        var j = 0;
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
          var j = 0;
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
