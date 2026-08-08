// Translated from solution.cpp.

var N = 5005;

var a = cpp_array(N);

var n: dynamic;

var cnt = cpp_array(N, N);

var ans = cpp_array(N);

var p = cpp_array(N);

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      var f = 1;
      if ((a[i] < 0))
      {
        f = -1;
        a[i] = (-a[i]);
      }
      {
        var j = 2;
        while (((j * j) <= a[i]))
        {
          while (((a[i] % ((j * j))) == 0))
          {
            a[i] /= (j * j);
          }
          j += 1;
        }
      }
      a[i] *= f;
      if ((a[i] == 0))
      {
        p[i] = i;
        cnt[i][i] = 0;
      } else
      {
        {
          var j = (i - 1);
          while (j)
          {
            if ((a[i] == a[j]))
            {
              p[i] = j;
              break;
            }
            j -= 1;
          }
        }
        cnt[i][i] = 1;
      }
      i += 1;
    }
  }
  ans[1] = n;
  {
    var l = 2;
    while ((l <= n))
    {
      {
        var i = 1;
        while ((((i + l) - 1) <= n))
        {
          var j = ((i + l) - 1);
          cnt[i][j] = cnt[i][(j - 1)];
          if ((p[j] < i))
          {
            cnt[i][j] += 1;
          }
          ans[max(1, cnt[i][j])] += 1;
          i += 1;
        }
      }
      l += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
}
