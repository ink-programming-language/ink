// Translated from solution.cpp.

func is_palindrom(a: dynamic) -> dynamic
{
  var s_reversed: dynamic = "";
  {
    var i: dynamic = (a.size() - 1);
    while ((i >= 0))
    {
      s_reversed += a[i];
      i -= 1;
    }
  }
  if ((a == s_reversed))
  {
    return true;
  }
  return false;
}

func main() -> dynamic
{
  var no: dynamic = "BCDEFGJKLNPQRSZ";
  var s: dynamic = cpp_uninitialized();
  read(s);
  if (((s.find_first_of(no) != -1) || (is_palindrom(s) == false)))
  {
    write("NO", "\n");
    return 0;
  }
  write("YES");
  return 0;
}
