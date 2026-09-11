// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  read(k);
  var a: dynamic = cpp_array(k);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ng: dynamic = 1;
  var ok: dynamic = 2;
  {
    var i: dynamic = (k - 1);
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
