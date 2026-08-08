// Translated from solution.cpp.

var int_cpp = dynamic;

func main()
{
  var k: dynamic;
  read(k);
  var a = cpp_array(k);
  {
    var i = 0;
    while ((i < k))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ng = 1;
  var ok = 2;
  {
    var i = (k - 1);
    while ((i >= 0))
    {
      ng = ((((1 + (ng / a[i]))) * a[i]) - 1);
      ok = ((((1 + (ok / a[i]))) * a[i]) - 1);
      if ((ng >= ok))
      {
        break;
      }
      i -= 1;
    }
  }
  if ((ng < ok))
  {
    write((ng + 1), cpp_char(" "), ok);
  } else
  {
    write(-1);
  }
  return 0;
}
