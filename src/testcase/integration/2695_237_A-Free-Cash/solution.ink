// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var h = cpp_array(n);
  var m = cpp_array(n);
  {
    int_cpp(i) = (0);
    while (((i) < (n)))
    {
      read(h[i], m[i]);
      (i) += 1;
    }
  }
  var maxi = 0;
  {
    int_cpp(i) = (0);
    while (((i) < (n)))
    {
      var a = 1;
      while (((((i + a) < n) && (h[i] == h[(i + a)])) && (m[i] == m[(i + a)])))
      {
        a += 1;
      }
      i += ((a - 1));
      maxi = max(maxi, a);
      (i) += 1;
    }
  }
  write(maxi, "\n");
}
