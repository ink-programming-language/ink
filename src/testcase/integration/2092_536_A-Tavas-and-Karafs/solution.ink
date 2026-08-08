// Translated from solution.cpp.

var INF = 1000000000;

var PI = 3.1415926535897932384626433832795028841971;

var sum = cpp_array(10000010);

var s = cpp_array(10000010);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var a: dynamic;
  var b: dynamic;
  var l: dynamic;
  var t: dynamic;
  var r: dynamic;
  var j: dynamic;
  var i: dynamic;
  read(a, b, n);
  {
    i = 1;
    while ((i <= 10000000))
    {
      s[i] = (a + (((i - 1)) * b));
      sum[i] = (sum[(i - 1)] + s[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      read(l, t, m);
      var left = l;
      var right = 10000000;
      var middle: dynamic;
      while (((left + 1) != right))
      {
        middle = (((left + right)) / 2);
        if ((middle < l))
        {
          left = middle;
          break;
        }
        if (((s[middle] <= t) && ((sum[middle] - sum[(l - 1)]) <= (t * m))))
        {
          left = middle;
        } else
        {
          right = middle;
        }
      }
      if ((!(((s[left] <= t) && ((sum[left] - sum[(l - 1)]) <= (t * m))))))
      {
        left = -1;
      }
      if ((left < l))
      {
        left = -1;
      }
      write(left, "\n");
      i += 1;
    }
  }
}
