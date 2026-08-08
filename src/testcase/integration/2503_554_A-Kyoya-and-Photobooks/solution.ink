// Translated from solution.cpp.

func main()
{
  var x = cpp_array(1000);
  read(x);
  var result = 0;
  {
    var i = 0;
    while ((i < (strlen(x) + 1)))
    {
      result += 25;
      i += 1;
    }
  }
  write((result + 1));
}
