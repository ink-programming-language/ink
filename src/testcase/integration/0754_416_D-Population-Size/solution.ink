// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var ans: dynamic;

var a = cpp_array(200005);

var d: dynamic;

func main()
{
  read(n);
  var i: dynamic;
  var j: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var k = 1;
  while ((k <= n))
  {
    ans += 1;
    {
      i = k;
      while ((a[i] == -1))
      {
        i += 1;
      }
    }
    {
      j = (i + 1);
      while ((a[j] == -1))
      {
        j += 1;
      }
    }
    if ((j > n))
    {
      break;
    }
    d = (((a[j] - a[i])) / ((j - i)));
    if (((((a[j] - a[i])) % ((j - i))) || ((a[j] - (d * ((j - k)))) <= 0)))
    {
      k = j;
      continue;
    }
    k = (j + 1);
    while ((((k <= n) && ((cpp_cast(a[j]) + (d * ((k - j)))) > 0)) && (((a[k] == -1) || (a[k] == (a[j] + (d * ((k - j)))))))))
    {
      k += 1;
    }
  }
  write(ans);
}
