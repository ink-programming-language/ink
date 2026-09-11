// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200005);

var d: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var k: dynamic = 1;
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
