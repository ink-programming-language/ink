// Translated from solution.cpp.

var v1: dynamic = cpp_uninitialized();

var v2: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(1000001);

func seive() -> dynamic
{
  var k: dynamic = 1000000;
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      arr[i] = true;
      i += 1;
    }
  }
  arr[0] = false;
  arr[1] = false;
  {
    var i: dynamic = 2;
    while ((i <= k))
    {
      if ((arr[i] == true))
      {
        {
          var j: dynamic = (2 * i);
          while ((j <= k))
          {
            arr[j] = false;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 3;
    while ((i <= k))
    {
      if ((arr[i] == true))
      {
        v1.push_back(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 5;
    while ((i <= k))
    {
      if ((arr[i] == true))
      {
        v2.push_back(i);
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n);
  k = ((n * ((n + 1))) / 2);
  write((k % 2), "\n");
  return 0;
}
