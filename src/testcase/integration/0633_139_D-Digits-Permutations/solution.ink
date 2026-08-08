// Translated from solution.cpp.

var N = (1e5 + 10);

var s = cpp_array(N);

var cnt1 = cpp_array(10);

var cnt2 = cpp_array(10);

var mi = cpp_array(5);

var ma: dynamic;

var mx = -1;

var c = [[5, 5], [1, 9], [2, 8], [3, 7], [4, 6]];

var cc = [[0, 9], [5, 4], [1, 8], [2, 7], [3, 6]];

var a1: dynamic;

var a2: dynamic;

func main()
{
  scanf("%s", s);
  var n = strlen(s);
  {
    var i = (0);
    while ((i < (n)))
    {
      cnt1[(s[i] - cpp_char("0"))] += 1;
      cnt2[(s[i] - cpp_char("0"))] += 1;
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (5)))
    {
      var a = c[i][0];
      var b = c[i][1];
      if (((cnt1[a] == 0) || (cnt2[b] == 0)))
      {
        i += 1;
        continue;
      }
      cnt1[a] -= 1;
      cnt2[b] -= 1;
      var t = 1;
      {
        var j = (0);
        while ((j < (10)))
        {
          var x = j;
          var y = (9 - j);
          t += min(cnt2[x], cnt1[y]);
          j += 1;
        }
      }
      if ((t > mx))
      {
        mx = t;
        ma = i;
      }
      cnt1[a] += 1;
      cnt2[b] += 1;
      i += 1;
    }
  }
  if ((mx == -1))
  {
    {
      var i = (0);
      while ((i < (10)))
      {
        {
          var j = (0);
          while ((j < (cnt1[i])))
          {
            a1 += (cpp_char("0") + i);
            j += 1;
          }
        }
        i += 1;
      }
    }
    a2 = a1;
  } else
  {
    var a = c[ma][0];
    var b = c[ma][1];
    cnt1[a] -= 1;
    cnt2[b] -= 1;
    a1 += (cpp_char("0") + a);
    a2 += (cpp_char("0") + b);
    {
      var j = (0);
      while ((j < (10)))
      {
        var x = j;
        var y = (9 - j);
        mi[j] = min(cnt1[x], cnt2[y]);
        {
          var k = (0);
          while ((k < (mi[j])))
          {
            a1 += (cpp_char("0") + x);
            k += 1;
          }
        }
        {
          var k = (0);
          while ((k < (mi[j])))
          {
            a2 += (cpp_char("0") + y);
            k += 1;
          }
        }
        cnt1[x] -= mi[j];
        cnt2[y] -= mi[j];
        j += 1;
      }
    }
    var zero = min(cnt1[0], cnt2[0]);
    {
      var j = (0);
      while ((j < (zero)))
      {
        a1 = (cpp_char("0") + a1);
        a2 = (cpp_char("0") + a2);
        j += 1;
      }
    }
    cnt1[0] -= zero;
    cnt2[0] -= zero;
    {
      var j = (0);
      while ((j < (10)))
      {
        {
          var k = (0);
          while ((k < (cnt1[j])))
          {
            a1 += (cpp_char("0") + j);
            k += 1;
          }
        }
        j += 1;
      }
    }
    {
      var j = (0);
      while ((j < (10)))
      {
        {
          var k = (0);
          while ((k < (cnt2[j])))
          {
            a2 += (cpp_char("0") + j);
            k += 1;
          }
        }
        j += 1;
      }
    }
  }
  reverse(a1.begin(), a1.end());
  reverse(a2.begin(), a2.end());
  write(a1, "\n", a2, "\n");
  return 0;
}
