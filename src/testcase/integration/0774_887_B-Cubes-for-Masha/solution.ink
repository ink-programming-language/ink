// Translated from solution.cpp.

var arr = cpp_array(15, 3);

var ar = cpp_array(100005);

func solve()
{
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic;
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < 6))
        {
          read(arr[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((n == 1))
  {
    {
      var i = 0;
      while ((i < 6))
      {
        ar[arr[0][i]] += 1;
        i += 1;
      }
    }
  }
  if ((n == 2))
  {
    {
      var i = 0;
      while ((i < 6))
      {
        {
          var j = 0;
          while ((j < 6))
          {
            var p = arr[0][i];
            var q = arr[1][j];
            ar[p] = 1;
            ar[q] = 1;
            ar[((p * 10) + q)] = 1;
            ar[((q * 10) + p)] = 1;
            j += 1;
          }
        }
        i += 1;
      }
    }
  } else
  {
    {
      var i = 0;
      while ((i < 6))
      {
        {
          var j = 0;
          while ((j < 6))
          {
            {
              var k = 0;
              while ((k < 6))
              {
                var p = arr[0][i];
                var q = arr[1][j];
                var r = arr[2][k];
                ar[p] = 1;
                ar[q] = 1;
                ar[r] = 1;
                ar[((p * 10) + q)] = 1;
                ar[((p * 10) + r)] = 1;
                ar[((q * 10) + p)] = 1;
                ar[((q * 10) + r)] = 1;
                ar[((r * 10) + p)] = 1;
                ar[((r * 10) + q)] = 1;
                ar[(((p * 100) + (q * 10)) + r)] += 1;
                ar[(((p * 100) + (r * 10)) + q)] += 1;
                ar[(((q * 100) + (p * 10)) + r)] += 1;
                ar[(((q * 100) + (r * 10)) + p)] += 1;
                ar[(((r * 100) + (p * 10)) + q)] += 1;
                ar[(((r * 100) + (q * 10)) + p)] += 1;
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
  var c = 0;
  {
    var i = 1;
    while ((i < 10000))
    {
      if ((ar[i] > 0))
      {
        c = i;
      } else
      {
        break;
      }
      i += 1;
    }
  }
  write(c, "\n");
}
