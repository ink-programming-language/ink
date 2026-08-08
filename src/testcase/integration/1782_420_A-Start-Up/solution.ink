// Translated from solution.cpp.

func is_palindrom(a: dynamic)
{
  var s_reversed = "";
  {
    var i = (a.size() - 1);
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

func main()
{
  var no = "BCDEFGJKLNPQRSZ";
  var s: dynamic;
  read(s);
  if (((s.find_first_of(no) != -1) || (is_palindrom(s) == false)))
  {
    write("NO", "\n");
    return 0;
  }
  write("YES");
  return 0;
}
