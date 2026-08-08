// Translated from solution.cpp.

func main()
{
  var test: dynamic;
  read(test);
  var name = cpp_array(50, 1010);
  var num = cpp_array(1010);
  var mx = 0;
  var m1: dynamic;
  var m2: dynamic;
  {
    var i = 0;
    while ((i < test))
    {
      read(name[i], num[i]);
      m1[name[i]] += num[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < test))
    {
      mx = max(mx, m1[name[i]]);
      i += 1;
    }
  }
  var j: dynamic;
  {
    j = 0;
    while ((j < test))
    {
      m2[name[j]] += num[j];
      if (((m1[name[j]] == mx) && (m2[name[j]] >= mx)))
      {
        break;
      }
      j += 1;
    }
  }
  write(name[j], "\n");
}
