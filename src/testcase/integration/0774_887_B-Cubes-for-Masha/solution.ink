// Translated from solution.cpp.

var arr: dynamic = cpp_array(15, 3);

var ar: dynamic = cpp_array(100005);

func solve() -> dynamic
{
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
      var i: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < 6))
      {
        {
          var j: dynamic = 0;
          while ((j < 6))
          {
            var p: dynamic = arr[0][i];
            var q: dynamic = arr[1][j];
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
      var i: dynamic = 0;
      while ((i < 6))
      {
        {
          var j: dynamic = 0;
          while ((j < 6))
          {
            {
              var k: dynamic = 0;
              while ((k < 6))
              {
                var p: dynamic = arr[0][i];
                var q: dynamic = arr[1][j];
                var r: dynamic = arr[2][k];
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
  var c: dynamic = 0;
  {
    var i: dynamic = 1;
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
