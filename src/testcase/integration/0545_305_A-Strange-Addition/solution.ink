// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(100);

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100, 100);

func Ok(x: dynamic, y: dynamic) -> dynamic
{
  while ((x && y))
  {
    var a: dynamic = (x % 10);
    var b: dynamic = (y % 10);
    x /= 10;
    y /= 10;
    if ((a && b))
    {
      return false;
    }
  }
  return true;
}

func main() -> dynamic
{
  read(n);
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if (Ok(arr[i], arr[j]))
          {
            a[i][j] = true;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var b: dynamic = false;
      {
        var j: dynamic = 0;
        while ((j < v.size()))
        {
          var k: dynamic = cpp_uninitialized();
          {
            k = 0;
            while ((k < v[j].size()))
            {
              if ((!a[i][v[j][k]]))
              {
                break;
              }
              k += 1;
            }
          }
          if ((k == v[j].size()))
          {
            v[j].push_back(i);
            b = true;
          }
          j += 1;
        }
      }
      if ((!b))
      {
        v.push_back(vector(1, i));
      }
      i += 1;
    }
  }
  {
    i = 0;
    j = 1;
    while ((j < v.size()))
    {
      if ((v[i].size() < v[j].size()))
      {
        i = j;
      }
      j += 1;
    }
  }
  write(v[i].size(), "\n");
  {
    j = 0;
    while ((j < v[i].size()))
    {
      write(arr[v[i][j]], " ");
      j += 1;
    }
  }
  return 0;
}
