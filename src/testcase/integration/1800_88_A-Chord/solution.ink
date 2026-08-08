// Translated from solution.cpp.

func main()
{
  var arr = cpp_array(10);
  var c = 0;
  var k = 0;
  var m = 0;
  var case1: dynamic;
  var case2: dynamic;
  var case3: dynamic;
  var case4: dynamic;
  var case5: dynamic;
  var case6: dynamic;
  var case7: dynamic;
  var case8: dynamic;
  var str: dynamic;
  var str1 = "CQDWEFRGTABH";
  getline(cin, str);
  {
    var i = 0;
    while ((i < str.size()))
    {
      if ((str[(i + 1)] == cpp_char("#")))
      {
        if ((str[i] == cpp_char("C")))
        {
          str[i] = cpp_char("Q");
        }
        if ((str[i] == cpp_char("D")))
        {
          str[i] = cpp_char("W");
        }
        if ((str[i] == cpp_char("F")))
        {
          str[i] = cpp_char("R");
        }
        if ((str[i] == cpp_char("G")))
        {
          str[i] = cpp_char("T");
        }
        c += 1;
      }
      {
        var j = 0;
        while ((j < str1.size()))
        {
          if ((str[i] == str1[j]))
          {
            arr[k] = j;
            k += 1;
            break;
          }
          j += 1;
        }
      }
      if ((c == 1))
      {
        i += 1;
        c = 0;
      }
      i = (i + 2);
    }
  }
  sort(arr, (arr + 3));
  while (true)
  {
    if ((((((arr[1] - arr[0]) == 3) || (((12 - ((arr[0] - arr[1]))) == 3)))) && ((((arr[2] - arr[1]) == 4) || (((12 - ((arr[1] - arr[2]))) == 4))))))
    {
      write("minor", "\n");
      m += 1;
      break;
    }
    if ((((((arr[1] - arr[0]) == 4) || (((12 - ((arr[0] - arr[1]))) == 4)))) && ((((arr[2] - arr[1]) == 3) || (((12 - ((arr[1] - arr[2]))) == 3))))))
    {
      write("major", "\n");
      m += 1;
      break;
    }
    if (!((next_permutation(arr, (arr + 3)))))
    {
      break;
    }
  }
  if ((m == 0))
  {
    write("strange", "\n");
  }
  return 0;
}
