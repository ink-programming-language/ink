// Translated from solution.cpp.

var a: dynamic = cpp_array(1001);

var mat: dynamic = cpp_array(21, 21);

var n2: dynamic = cpp_uninitialized();

var n4: dynamic = cpp_uninitialized();

var n1: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= (n * n)))
    {
      read(t);
      a[t] += 1;
      i += 1;
    }
  }
  var odd: dynamic = 0;
  var oddi: dynamic = cpp_uninitialized();
  var r: dynamic = 1;
  var c: dynamic = 1;
  if (((n % 2) == 0))
  {
    {
      var i: dynamic = 1;
      while ((i <= 1000))
      {
        while ((a[i] >= 4))
        {
          n4.push(i);
          a[i] -= 4;
          if ((n4.size() == ((n * n) / 4)))
          {
            break;
          }
        }
        if ((n4.size() == ((n * n) / 4)))
        {
          break;
        }
        i += 1;
      }
    }
    if ((n4.size() == ((n * n) / 4)))
    {
      write("YES", "\n");
    } else
    {
      write("NO", "\n");
      return 0;
    }
    {
      var i: dynamic = 1;
      while ((i <= (n / 2)))
      {
        {
          var j: dynamic = 1;
          while ((j <= (n / 2)))
          {
            var t: dynamic = n4.front();
            n4.pop();
            mat[i][j] = t;
            mat[((n - i) + 1)][j] = t;
            mat[i][((n - j) + 1)] = t;
            mat[((n - i) + 1)][((n - j) + 1)] = t;
            j += 1;
          }
        }
        i += 1;
      }
    }
  } else
  {
    var x4: dynamic = ((((n - 1)) * ((n - 1))) / 4);
    var x2: dynamic = ((n - 1));
    {
      var i: dynamic = 1;
      while ((i <= 1000))
      {
        while ((a[i] >= 4))
        {
          n4.push(i);
          a[i] -= 4;
          if ((n4.size() == x4))
          {
            break;
          }
        }
        if ((n4.size() == x4))
        {
          break;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= 1000))
      {
        while ((a[i] >= 2))
        {
          n2.push(i);
          a[i] -= 2;
          if ((n2.size() == x2))
          {
            break;
          }
        }
        if ((n2.size() == x2))
        {
          break;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= 1000))
      {
        if (a[i])
        {
          n1 = i;
          break;
        }
        i += 1;
      }
    }
    if (((n4.size() != x4) || (n2.size() != x2)))
    {
      write("NO");
      return 0;
    } else
    {
      write("YES", "\n");
      {
        var i: dynamic = 1;
        while ((i <= (n / 2)))
        {
          {
            var j: dynamic = 1;
            while ((j <= (n / 2)))
            {
              var t: dynamic = n4.front();
              n4.pop();
              mat[i][j] = t;
              mat[((n - i) + 1)][j] = t;
              mat[i][((n - j) + 1)] = t;
              mat[((n - i) + 1)][((n - j) + 1)] = t;
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= (n / 2)))
        {
          var t: dynamic = n2.front();
          n2.pop();
          mat[i][((n / 2) + 1)] = t;
          mat[((n - i) + 1)][((n / 2) + 1)] = t;
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= (n / 2)))
        {
          var t: dynamic = n2.front();
          n2.pop();
          mat[((n / 2) + 1)][i] = t;
          mat[((n / 2) + 1)][((n - i) + 1)] = t;
          i += 1;
        }
      }
      mat[((n / 2) + 1)][((n / 2) + 1)] = n1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          write(mat[i][j], cpp_char(" "));
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}
