// Translated from solution.cpp.

var a = cpp_array(1001);

var mat = cpp_array(21, 21);

var n2: dynamic;

var n4: dynamic;

var n1: dynamic;

func main()
{
  var n: dynamic;
  var t: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= (n * n)))
    {
      read(t);
      a[t] += 1;
      i += 1;
    }
  }
  var odd = 0;
  var oddi: dynamic;
  var r = 1;
  var c = 1;
  if (((n % 2) == 0))
  {
    {
      var i = 1;
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
      var i = 1;
      while ((i <= (n / 2)))
      {
        {
          var j = 1;
          while ((j <= (n / 2)))
          {
            var t = n4.front();
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
    var x4 = ((((n - 1)) * ((n - 1))) / 4);
    var x2 = ((n - 1));
    {
      var i = 1;
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
      var i = 1;
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
      var i = 1;
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
        var i = 1;
        while ((i <= (n / 2)))
        {
          {
            var j = 1;
            while ((j <= (n / 2)))
            {
              var t = n4.front();
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
        var i = 1;
        while ((i <= (n / 2)))
        {
          var t = n2.front();
          n2.pop();
          mat[i][((n / 2) + 1)] = t;
          mat[((n - i) + 1)][((n / 2) + 1)] = t;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= (n / 2)))
        {
          var t = n2.front();
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
