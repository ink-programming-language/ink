// Translated from solution.cpp.

var v1: dynamic;

var v2: dynamic;

var arr = cpp_array(1000001);

func seive()
{
  var k = 1000000;
  {
    var i = 1;
    while ((i <= k))
    {
      arr[i] = true;
      i += 1;
    }
  }
  arr[0] = false;
  arr[1] = false;
  {
    var i = 2;
    while ((i <= k))
    {
      if ((arr[i] == true))
      {
        {
          var j = (2 * i);
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
    var i = 3;
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
    var i = 5;
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

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n);
  k = ((n * ((n + 1))) / 2);
  write((k % 2), "\n");
  return 0;
}
