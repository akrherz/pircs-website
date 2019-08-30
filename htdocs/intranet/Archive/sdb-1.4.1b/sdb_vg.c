/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * COPYING file.                                                            *
 *                                                                          *
 ****************************************************************************/
#ifdef RCSID
static char RcsId[] = "@(#)sdb_vg.c,v 1.25 1996/04/15 17:58:26 georgev Exp";
#endif

/* sdb_vg.c,v 1.25 1996/04/15 17:58:26 georgev Exp */

/*
   LIST Functions
   --------------
    add_vg         - Add Vgroup to List with element List       
    add_elem       - Add element to Vgroup element list
    vg_cmp         - Iterator fcn to find the Vgroup given the ref
    unset_toplevel - Iterator fcn to set this Vgroup as not a toplevel one
    is_toplevel    - Iterator fcn to indiate if this Vgroup is a toplevel one 
    not_toplevel   - Iterator fcn to indiate if this Vgroup is not a toplevel one
    free_elem      - Frees element from Vgroup element list(Iterator)
    free_vg        - Frees Vgroup element and Vgroup element list (Iterator)
    cleanup_vgs    - Frees Vgroup Lists

   Regular Functions
   -----------------
    do_vgs      - Process Vgroups in file and add to List       
    dump_vg     - dump info on Vgroup given its ref
    dump_vgs    - print out the info for toplevel Vgroups in the file
    show_vgtree - prints Vgroup and elements
    show_tree   - prints a tree representation of the objects in the HDF file

   OBSOLETE
   --------
   vgdumpfull   - display the contents of a vgroup
   print_tree   - print the tree structure of an HDF file
   tree         - create tree representation of HDF objects in file

 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_vg.h"
#include "sdb_sds.h"
#include "sdb_util.h"
#include "glist.h"   /* Generic Lists */

/* ------------------------------------------------------------------------
 NAME
     add_vg - Add Vgroup to List with element List       
 DESCRIPTION
     Add Vgroup to List with element List  
 RETURNS 
     SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
add_vg(Generic_list *vg_list,    /* Vgroup List */
       intn toplevel,            /* flag */
       intn nelem,               /* number of elements in Vgroup */
       int32 vref,               /* ref of Vgroup */
       char *name,               /* name of Vgroup */
       char *class,              /* class of Vgroup */
       Generic_list *elem_list,  /* element List */
       lvar_st *l_vars)
{
    tvg_st *vg_elem = NULL;
    int ret_value = SUCCEED;

    /* allocate space for Vgroup element */
    if ((vg_elem = (tvg_st *)HDmalloc(sizeof(tvg_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_vg: failed to allocate space for vg_elem\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* allocate space for Vgroup name */
    if(name != NULL)
      {
          if ((vg_elem->name = (char *)HDmalloc(sizeof(char)*(HDstrlen(name)+1))) == NULL)
            {
                gateway_err(l_vars->hfp,"add_vg: failed to allocate space for vg_elem->name\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }
          HDstrcpy(vg_elem->name, name);
      }
    else
        vg_elem->name = NULL;

    /* allocate space for Vgroup class */
    if(class != NULL)
      {
          if ((vg_elem->class = (char *)HDmalloc(sizeof(char)*(HDstrlen(class)+1))) == NULL)
            {
                gateway_err(l_vars->hfp,"add_vg: failed to allocate space for vg_elem->class\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }
          HDstrcpy(vg_elem->class, class);
      }
    else
        vg_elem->class = NULL;

    /* fill in relevant Vgroup info */
    vg_elem->toplevel = toplevel;
    vg_elem->vref     = vref;
    vg_elem->nelem     = nelem;
    vg_elem->elem_list = elem_list;
    vg_elem->parent    = NULL;

    /* add to list */
    add_to_beginning(*vg_list, vg_elem);
    DBUG_PRINT(3,(LOGF,"add_vg: adding vgroup,vref=%d,name=%s,class=%s,nelem=%d \n",
                  vg_elem->vref,vg_elem->name,vg_elem->class,vg_elem->nelem));

  done:
    /* failure */
    if (ret_value == FAIL)
      {
          if (vg_elem != NULL)
            {
                if(vg_elem->name != NULL)
                    HDfree(vg_elem->name);
                if(vg_elem->class != NULL)
                    HDfree(vg_elem->class);
                HDfree(vg_elem);
            }
      }
    return ret_value;
} /* add_vg() */

/* ------------------------------------------------------------------------
 NAME
     add_elem - Add element to Vgroup element list
 DESCRIPTION
     Add element to Vgroup element list  
 RETURNS 
     SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
add_elem(Generic_list *elem_list, /* Vgroup element list */
         uint16 tag,              /* tag of element */
         uint16 ref,              /* ref of element */
         lvar_st *l_vars)
{
    telem_st *el_elem  = SUCCEED;
    int      ret_value = SUCCEED;

    /* allocate space for element */
    if ((el_elem = (telem_st *)HDmalloc(sizeof(telem_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_elem: failed to allocate space for el_elem\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   
    /* fille in relvant info */
    el_elem->ref = ref;
    el_elem->tag = tag;
    el_elem->name = NULL;

    /* add to list */
    add_to_beginning(*elem_list, el_elem);
    DBUG_PRINT(3,(LOGF,"add_elem: adding elem,tag=%d, ref=%d \n",
                  el_elem->tag, el_elem->ref));

  done:
    return ret_value;
} /* add_elem() */

/* ------------------------------------------------------------------------
 NAME
     vg_cmp - Iterator fcn to find the Vgroup given the ref
 DESCRIPTION
     Iterator fcn to find the Vgroup given the ref
 RETURNS 
     1->SUCCESS, 0->FAIL
--------------------------------------------------------------------------*/
int
vg_cmp(void *dptr, /* Vgroup element */
       void *args)
{
    tvg_st *vg_elem = dptr;
    uint16 *ref = args;

    if(vg_elem->vref == *ref)
        return 1;
    else
        return 0;
} /*vg_cmp() */

/* ------------------------------------------------------------------------
 NAME
     unset_toplevel - Iterator fcn to set this Vgroup as not a toplevel one
 DESCRIPTION
     Iterator fcn to set this Vgroup as not a toplevel one
 RETURNS 
     Nothing
--------------------------------------------------------------------------*/
void
unset_toplevel(void *dptr, /* Vgroup element */
               void *args)
{
    tvg_st *vg_elem = dptr;
    uint16 *ref     = args;

    if(vg_elem->vref == *ref)
        vg_elem->toplevel = 0;
} /* unset_top_level() */

/* ------------------------------------------------------------------------
 NAME
     is_toplevel - Iterator fcn to indiate if this Vgroup is a toplevel one 
 DESCRIPTION
     Iterator fcn to indiate if this Vgroup is a toplevel one 
 RETURNS 
     1->SUCCESS, 0->FAIL
--------------------------------------------------------------------------*/
int
is_toplevel(void *dptr, /* Vgroup element */
            void *args)
{
    tvg_st *vg_elem = dptr;

    if(vg_elem->toplevel)
        return 1;
    else
        return 0;
} /* is_toplevel() */

/* ------------------------------------------------------------------------
 NAME
     not_toplevel - Iterator fcn to indiate if this Vgroup is not a toplevel one
 DESCRIPTION
     Iterator fcn to indiate if this Vgroup is not a toplevel one
 RETURNS 
     1->SUCCESS, 0->FAIL
--------------------------------------------------------------------------*/
int
not_toplevel(void *dptr, /* Vgroup element */
             void *args)
{
    tvg_st *vg_elem = dptr;

    if(!vg_elem->toplevel)
        return 1;
    else
        return 0;
}/* not_toplevel() */

/* ------------------------------------------------------------------------
 NAME
     free_elem - Frees element from Vgroup element list(Iterator)
 DESCRIPTION
     Frees element from Vgroup element list
 RETURNS 
     Nothing
--------------------------------------------------------------------------*/
void
free_elem(void *dptr, /* elment of Vgroup elment list to free */
          void *args)
{
    telem_st *el_elem = dptr;
    
    /* free structure itself */
    if(el_elem != NULL)
      {
          if(el_elem->name != NULL)
              free(el_elem->name);
          free(el_elem);
      }
} /*free_elem() */

/* ------------------------------------------------------------------------
 NAME
     free_vg  - Frees Vgroup element and Vgroup element list (Iterator)
 DESCRIPTION
     Frees Vgroup element and Vgroup element list (Iterator)
 RETURNS 
     Nothing
--------------------------------------------------------------------------*/
void
free_vg(void *dptr,  /* Vgroup element to free */
        void *args)
{
    tvg_st *vg_elem = dptr;

    if (vg_elem != NULL)
      {
          /* free elem list */
          if (vg_elem->elem_list != NULL)
            {
                perform_on_list(*(vg_elem->elem_list),free_elem,NULL);
                /* destroy element list */
                destroy_list(vg_elem->elem_list);
                free(vg_elem->elem_list);
            }

          /* free name and class */
          if (vg_elem->name != NULL)
              HDfree(vg_elem->name);
          if (vg_elem->class != NULL)
              HDfree(vg_elem->class);

          /* free structure itself */
          free(vg_elem);
      }
} /* free_vg() */

/* ------------------------------------------------------------------------
 NAME
     cleanup_vgs - Frees Vgroup Lists
 DESCRIPTION
      Frees Vgroup Lists
 RETURNS 
      Nothing
--------------------------------------------------------------------------*/
void
cleanup_vgs(lvar_st *l_vars)
{
    ENTER(2,"cleanup_vgs");
    /* top level Vgroup list */
    if (l_vars->tvg_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_vgs: destroy tvg_list=%d\n", 
                        num_of_objects(*(l_vars->tvg_list))));
          /* destory list itself */
          destroy_list(l_vars->tvg_list);
          free(l_vars->tvg_list);
          l_vars->tvg_list = NULL;
      }

    /* non toplevel Vgrop list */
    if (l_vars->ovg_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_vgs: destroy ovg_list=%d\n", 
                        num_of_objects(*(l_vars->ovg_list))));
          /* destory list itself */
          destroy_list(l_vars->ovg_list);
          free(l_vars->ovg_list);
          l_vars->ovg_list = NULL;
      }

    /* now the Vgroup list */
    if (l_vars->vg_list != NULL)
      { /* free elements first */
          DBUG_PRINT(1,(LOGF,"cleanup_vgs: destroy vg_list=%d\n", 
                        num_of_objects(*(l_vars->vg_list))));
          perform_on_list(*(l_vars->vg_list),free_vg,NULL);        
          /* destory list itself */
          destroy_list(l_vars->vg_list);

          free(l_vars->vg_list);
          l_vars->vg_list = NULL;
      }
    EXIT(2,"cleanup_vgs");
}/* cleanup_vgs() */

/* ------------------------------------------------------------------------
 NAME
       do_vgs - Process Vgroups in file and add to List       
 DESCRIPTION
       Process Vgroups in file and add to List
 RETURNS 
       SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
do_vgs(char *fname,     /* file name */ 
       lvar_st *l_vars)
{
    int32 i, j;
    int32 count;                /* number of elements in Vgroup */
    int32 nvgs = 0;
    int32 vg;                    /* Vgroup handle */
    int32 tag;                   /* tag of element */
    int32 ref;                   /* reference number of element */
    int32 myref;                 /* reference numbero of ? */
    int32 fid = -1;
    char name[VSNAMELENMAX+1];     /* Vgroup name */
    char class[VSNAMELENMAX+1];    /* Vgroup class */
    char fields[(FIELDNAMELENMAX+1)*VSFIELDMAX];
    int32 vd;
    int32 intr;
    int32 sz;
    int32 cnt;
    FILE *h_fp = l_vars->hfp;
    char tmpbuf[128];
    int vcnt;
    Generic_list  *vg_elemlist = NULL; /* pointer to element list */
    Generic_list  gtmp;
    tvg_st    *vg_elem = NULL;
    telem_st  *el_elem = NULL;
    int ret_value = SUCCEED;

    ENTER(2,"do_vgs");
    /* Open file */
    if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_vgs: Problem opening HDF file",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

    /* find the number of Vgroups in the file */
    if((vcnt = Hnumber(fid, DFTAG_VG)) < 1) 
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* setup to start accessing Vgroups */
    if (Vstart(fid) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_vgs: error with Vstart ",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

    /* initialize Vroup lists */
    if(l_vars->vg_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->vg_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_vgs: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->vg_list);
      }

    /* look through all other VGroups */
    myref = -1;
    for(myref = Vgetid(fid, myref), i=0; myref != FAIL && i < vcnt; 
        myref = Vgetid(fid, myref), i++) 
      {
          /* Attach to Vgroup */
          if ((vg = Vattach(fid, myref, "r")) == FAIL)
            {
                gateway_err(l_vars->hfp,"do_vgs: failed to attach Vgroup \n",0,l_vars);
                continue;
            }  

          /* Get Vgroup name, class and number of elements */
          if ((Vgetname(vg, name))== FAIL)
            {
                Vdetach(vg); /* Dettach from Vgroup */
                gateway_err(l_vars->hfp,"do_vgs: failed to get Vgroup name\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          if ((Vgetclass(vg, class)) == FAIL)
            {
                Vdetach(vg); /* Dettach from Vgroup */
                gateway_err(l_vars->hfp,"do_vgs: failed to get Vgroup class\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }  

          DBUG_PRINT(3,(LOGF,"do_vgs: testing vgroup class=%s,len=%d \n", class,HDstrlen(class)));
          /* is this Vgroup part of an SDS */
          if (!HDstrncmp(class,"CDF0.0",HDstrlen("CDF0.0"))
              || !HDstrncmp(class,"Attr0.0",HDstrlen("Attr0.0"))
              || !HDstrncmp(class,"Var0.0",HDstrlen("Var0.0"))
              || !HDstrncmp(class,"Dim0.0",HDstrlen("Dim0.0"))
              || !HDstrncmp(class,"UDim0.0",HDstrlen("UDim0.0"))
              || !HDstrncmp(class,"DimVal0.0",HDstrlen("DimVal0.0"))
              || !HDstrncmp(class,"Data0.0",HDstrlen("Data0.0")))
            {/* Yes - Ignore Vgroups belonging to SDS's */
                Vdetach(vg); /* Dettach from Vgroup */
                continue;
            }

          /* Find number of elements in Vgroup */
          count = Vntagrefs(vg);

          if (count > 0)
            {
                /* allocate list to hold elements */
                if ((vg_elemlist = HDmalloc(sizeof(Generic_list))) == NULL)
                  {
                      Vdetach(vg); /* Dettach from Vgroup */
                      gateway_err(l_vars->hfp,"do_vgs: failed to allocate space for attribute list\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }   

                /* initialize attriube list of this SDS */
                initialize_list(vg_elemlist);

                for(j = 0; j < count; j++) 
                  { /* Process through elements of Vgroup */
                      if ((Vgettagref(vg, j, &tag, &ref)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"do_vgs: failed to get Vgroup tag/ref \n",0,l_vars);
                            continue;
                        }  

                      /* add to element list of this Vgorup */
                      if (add_elem(vg_elemlist,tag,ref,l_vars) == FAIL)
                        {
                            Vdetach(vg); /* Dettach from Vgroup */
                            gateway_err(h_fp,"do_vgs: failed to add elem to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                  }
            }
          else
              vg_elemlist = NULL;

          /* add Vgrup w/ element list to Vgroup list */
          if (add_vg(l_vars->vg_list,1,count,myref,name,class,
                     vg_elemlist,l_vars) == FAIL)
            {
                Vdetach(vg); /* Dettach from Vgroup */
                gateway_err(h_fp,"do_vgs: failed to add vgroup to list \n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          Vdetach(vg); /* Dettach from Vgroup */
      } /* end for all Vgroups */

    /* process throught all Vgroups and their element lists */
    gtmp = copy_list(*(l_vars->vg_list));
    reset_to_beginning(*(l_vars->vg_list));
    nvgs = num_of_objects(*(l_vars->vg_list));
    DBUG_PRINT(3,(LOGF,"do_vgs:processing %d Vgroups to find non-toplevel Vgroup \n",nvgs));
    for (i = 0; i < nvgs ; i++)
      {
          tvg_st *vtmp = NULL;
          tsds_st *sds_elem = NULL;
          int32 sdref;

          vg_elem = next_in_list(*(l_vars->vg_list));
          DBUG_PRINT(3,(LOGF,"do_vgs: i=%d,processing Vgroup %s\n",
                        i,vg_elem->name));
          for (j = 0; j < vg_elem->nelem ; j++)
            {
                el_elem = next_in_list(*(vg_elem->elem_list));

                /* Check to see if it is a Vgroup */
                switch (el_elem->tag)
                  {
                  case DFTAG_VG:
                      DBUG_PRINT(3,(LOGF,"do_vgs:found non-toplevel Vgroup \n"));
                      /* Set in to not toplevel in  overall Vgroup list */
                      /* set parent of this Vgroup */
                      reset_to_beginning(gtmp);
                      vtmp = first_that(gtmp,vg_cmp,&(el_elem->ref));
                      if(vtmp != NULL)
                        {
                            DBUG_PRINT(3,(LOGF,"do_vgs:non-toplevel Vgroup %s\n",
                                          vtmp->name));
                            vtmp->toplevel = 0;
                            vtmp->parent = vg_elem;
                            /* set name */
                            el_elem->name = HDstrdup(vg_elem->name);
                        }
                      break;
                  case DFTAG_VH: /* Vdata */
                      /* attach to Vdata */
                      if ((vd = VSattach(fid, el_elem->ref, "r")) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"do_vgs: failed to open Vdata \n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }  

                      /* get Vdata name */
                      if ((VSinquire(vd, &cnt, &intr, fields, &sz, name)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"do_vgs: failed to get info on Vdata \n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   
                      
                      /* set element name */
                      el_elem->name = HDstrdup(name);

                      VSdetach(vd); /* deatach from Vdata */
                      break;
                  case DFTAG_NDG:
                      /* set SDS to not being lone */
                      sdref = el_elem->ref;
                      DBUG_PRINT(3,(LOGF,"do_vgs:found non-lone SDS,ref=%d \n",
                                    sdref));
                      /* find correct sds */
                      reset_to_beginning(*(l_vars->sds_list));
                      sds_elem = first_that(*(l_vars->sds_list),ndg_cmp,&sdref);

                      if (sds_elem != NULL)
                        {   /* set to NOT lone */
                            sds_elem->lone = 0;
                            /* set name */
                            el_elem->name = HDstrdup(sds_elem->name);
                        }
                      break;
                  default: /* get name */
                      if (HDgettagsname(el_elem->tag) != NULL)
                        {
                            el_elem->name = HDstrdup(HDgettagsname(el_elem->tag));
                        }
                      else /* non-hdf object */
                        {
                            sprintf(tmpbuf, "<OPTION> Unknown Object tag=%d,ref=%d ",
                                    el_elem->tag, el_elem->ref);		   
                            el_elem->name = HDstrdup(tmpbuf);
                        }
                      break;
                  }
            } /* end for element count */
      } /* end for vgroups */
    
    /* destroy copy of Vgroup list */
    destroy_list(&gtmp);

    /* initialize top level Vroup lists */
    if(l_vars->tvg_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->tvg_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_vgs: failed to allocate space for top level  list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   
      }

    /* get list of top level Vgrops */
    *l_vars->tvg_list = all_such_that(*(l_vars->vg_list),is_toplevel,NULL);


    /* initialize NON-top level Vroup lists */
    if(l_vars->ovg_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->ovg_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_vgs: failed to allocate space for Non top level  list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   
      }
    
    /* get list of non-toplvel Vgroups */
    *l_vars->ovg_list = all_such_that(*(l_vars->vg_list),not_toplevel,NULL);

  done:
    /* failure */
    if(ret_value == FAIL)
      {
          cleanup_vgs(l_vars);
      }

    /* clean up */
    if (fid != FAIL)
      {
          Vend(fid);
          Hclose(fid);
      }
    EXIT(2,"do_vgs");
    return ret_value;
} /* do_vgs() */

/* ------------------------------------------------------------------------
 NAME
      dump_vg - dump info on Vgroup given its ref
 DESCRIPTION
      dump info on Vgroup given its ref
 RETURNS 
      SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
dump_vg(char *fname,     /* file name */
        uint16 vref,     /* ref of Vgroup */
        lvar_st *l_vars)
{
    int32 i,j;
    int32 k;
    int32 cnt;
    int32 vd;
    int32 tag;                   /* tag of element */
    int32 ref;                   /* reference number of element */
    int32 intr;
    int32 sz;
    int32 count;
    int32 fid = -1;
    char fields[(FIELDNAMELENMAX+1)*VSFIELDMAX];
    char name[VSNAMELENMAX+1];     /* Vgroup name */
    FILE *h_fp;
    tvg_st    *vg_elem = NULL;
    tvg_st    *vgtmp = NULL;
    telem_st  *el_elem = NULL;
    tsds_st   *sds_elem = NULL;
    int       ret_value = SUCCEED;

    ENTER(2,"dump_vg");
    DBUG_PRINT(1,(LOGF, "dump_vg: find Vgroup whose ref=%d : \n", 
                  vref));

    h_fp = l_vars->hfp;
    if (l_vars->vg_list != NULL)
      {
          /* get Vgroup element from list */
          reset_to_beginning(*(l_vars->vg_list));
          vg_elem = first_that(*(l_vars->vg_list),vg_cmp,&vref);

          count = vg_elem->nelem;
        
          /* set up title and stuff */
          fprintf(l_vars->hfp, "<TITLE>Vgroup: %s</TITLE>\n",vg_elem->name);
          fprintf(l_vars->hfp, "<H1>Vgroup: %s</H1>\n", vg_elem->name);

#if 0        
          fprintf(l_vars->hfp, "This Vgroup is named %s and is of class <B>%s</B>.\n", 
                  vg_elem->name, vg_elem->class);
#endif
          /* get Table stuff ready */
          fprintf(h_fp, "<TABLE BORDER>\n");
          fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH> <TH> %s </TH>", 
                  "Vgroup Name", "Vgroup Class", "Number of Elements");

          fprintf(h_fp, "<TR ALIGN=\"center\" VALIGN=\"center\"><TD> <pre> %s </pre> </TD> <TD> <pre> %s </pre> </TD> ", 
                  vg_elem->name, (vg_elem->class[0] ? vg_elem->class : "--"));
          fprintf(h_fp, "<TD> <pre> %d </pre> </TD><TR>", 
                  vg_elem->nelem);
          fprintf(h_fp, "</TABLE> <p>\n");
          DBUG_PRINT(1,(LOGF, "dump_vg: This Vgroup is named %s and is of class %s.\n", 
                        vg_elem->name, vg_elem->class));
          DBUG_PRINT(1,(LOGF, "dump_vg: There are %d elements in Vgroup %s : \n", 
                        count, vg_elem->name));

          /* If no elements just dettach and return */
          if(count == 0) 
            {
                ret_value = SUCCEED;
                goto done;
            }

#if 0
          if(count == 1)
              fprintf(l_vars->hfp, "There is %d element in Vgroup %s : \n", 
                      count, vg_elem->name);
          else
              fprintf(l_vars->hfp, "There are %d elements in Vgroup %s : \n", 
                      count, vg_elem->name);
          DBUG_PRINT(1,(LOGF, "There are %d elements in Vgroup %s : \n", 
                        count, vg_elem->name));
#endif

          fprintf(l_vars->hfp, "<FORM METHOD=\"POST\" ");
          fprintf(l_vars->hfp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                  l_vars->h_env->path_info);
          fprintf(l_vars->hfp, "<SELECT NAME=\"Vgroups\" SIZE=5>\n");

          reset_to_beginning(*(vg_elem->elem_list));
          for(i = 0; i < count; i++) 
            { /* Get tag/ref of element */
                el_elem = next_in_list(*(vg_elem->elem_list));
                tag = el_elem->tag;
                ref = el_elem->ref;

                /* Switch on TAG */
                switch((uint16) tag) 
                  {
                  case DFTAG_VG: /* Vgroups */
                      if (i==0) 
                          fprintf(l_vars->hfp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                  DFTAG_VG, ref, 0);
                      else
                          fprintf(l_vars->hfp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                  DFTAG_VG, ref, 0);

                      /* Get Vgroup name */
                      vgtmp = first_that(*(l_vars->vg_list),vg_cmp,&(el_elem->ref));

                      /* We don't correctly handle display Vgroups which are
                         SDS i.e. class 'CDF 0.0' as SDS */
                      if (vgtmp != NULL)
                        {
                            DBUG_PRINT(1,(LOGF, "  an element of the Vgroup is a Vgroup named %s and is of class %s.\n", 
                                          vgtmp->name, vgtmp->class));
                            fprintf(l_vars->hfp, "Vgroup: %s\n", vgtmp->name);
                        }
                      break;
                  case DFTAG_VH: /* Vdata */
                      /* Open file */
                      if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"dump_vg: Problem opening HDF file",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }

                      /* setup to start accessing Vgroups */
                      if (Vstart(fid) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"dump_vg: error with Vstart ",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }

                      /* attach to Vdata */
                      if ((vd = VSattach(fid, ref, "r")) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"dump_vg: failed to open Vdata \n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }  

                      /* get Vdata name */
                      if ((VSinquire(vd, &cnt, &intr, fields, &sz, name)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"dump_vg: failed to get info on Vdata \n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   
                      if (i==0) 
                          fprintf(l_vars->hfp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                  DFTAG_VH, ref, 0);
                      else
                          fprintf(l_vars->hfp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                  DFTAG_VH, ref, 0);
                      fprintf(l_vars->hfp, "Vdata: %s\n", name);

                      VSdetach(vd); /* deatach from Vdata */
                      /* clean up */
                      Vend(fid);
                      Hclose(fid);
                      break;
                  case DFTAG_NDG: /* NDG i.e.SDS */
                      if (i==0) 
                          fprintf(l_vars->hfp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                  DFTAG_NDG, ref, 0);
                      else
                          fprintf(l_vars->hfp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                  DFTAG_NDG, ref, 0);

                      /* find correct sds */
                      sds_elem = first_that(*(l_vars->sds_list),ndg_cmp,&ref);

                      if (sds_elem != NULL)
                        {
                            fprintf(l_vars->hfp, "%s\n", sds_elem->name);
#if 0
                            fprintf(l_vars->hfp, "Scientific Dataset(%s)\n", HDgettagsname(tag));
#endif
                        }
                      break;
                  case DFTAG_RIG:
                  case DFTAG_RI8:
                  {
                      int32 status;   /* flag: function return? */
                      int32 w;        /* width of image */
                      int32 h;        /* heigth of image */
                      intn  ip;       /* flag: paletter? */
                    
                      /* Set up to read image */
                      if ((DFR8readref(fname, (uint16) ref)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"dump_vg: failed set ref of Image \n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }  

                      /* Get dimensions of image */
                      if ((status = DFR8getdims(fname, &w, &h, &ip)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"dump_vg: failed get dims of Image \n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }  

                      fprintf(l_vars->hfp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                              DFTAG_RIG, ref, 0);
                      fprintf(l_vars->hfp, "Raster Image(%dx%d)\n", w,h);
                    
                  }
                  break;
                  default: /* print tag name */
                      if (HDgettagsname(tag) != NULL)
                        {
                            fprintf(l_vars->hfp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                                    tag, ref, 0);		   
                            fprintf(l_vars->hfp, "%s\n", HDgettagsname(tag));
                        }
                      else
                          fprintf(l_vars->hfp, "<OPTION> Unknown Object tag=%d,ref=%d ",
                                  tag, ref);		   
                      /* Must be an object we don't handle so just display  tag/ref */
                      break;
                  } /* end switch "tag" */
            } /* end for loop for each element in Vgroup */

          if (!l_vars->do_dump)
            {
                fprintf(l_vars->hfp, "</SELECT> <P>");
                fprintf(l_vars->hfp, "To select a particular vgroup element, press this button: ");
                fprintf(l_vars->hfp, "<INPUT TYPE=\"submit\" VALUE=\"Select Element\">. <P>\n");
                fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",
                        l_vars->f_name); 
                fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                        2);
                fprintf(l_vars->hfp, "</FORM>");
            }

      }

  done:
    if (ret_value == FAIL)
      {
      }
    EXIT(2,"dump_vg");
    return ret_value;
} /* dump_vg */

/*------------------------------------------------------------------------- 
 NAME
       dump_vgs - print out the info for toplevel Vgroups in the file
 DESCRIPTION
       print out the info for toplevel Vgroups in the file
 RETURNS
       SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
dump_vgs(char *fname,    /* file name */
         lvar_st *l_vars)
{
    int32   i;
    char    tmp_html[1024];
    FILE    *h_fp;
    tvg_stp  tvgelem = NULL;         /* pointer to Vgroup element */
    intn     ntvg;
    int      ret_value = SUCCEED;

    ENTER(2,"dump_vgs");

    if(l_vars->vg_list != NULL)
      {
          /* do we have top level Vgroups */
          ntvg = num_of_objects(*(l_vars->tvg_list));
          if(ntvg)
            { /* Yes, we have some top level Vgroups */
                if (l_vars->do_dump)
                  {
                      /* Create name for HTML file */
                      if (l_vars->html_dir == NULL)
                          sprintf(tmp_html,"%s_vg.html",fname);
                      else
                          sprintf(tmp_html,"%s/%s_vg.html",l_vars->html_dir,l_vars->f_name);
            
                      DBUG_PRINT(1,(LOGF,"dump_vgs: hdf html file name %s \n", tmp_html));
          
                      /* Open temproary file to write HTML description of 
                         HDF/netCDF file */
                      if (!(h_fp = fopen(tmp_html, "w")))
                        {
                            ret_value = FAIL; 
                            goto done;
                        }

                      /* Write MIME header */
                      if (write_html_header(h_fp, TEXT_HTML,l_vars) == FAIL)
                        {
                            gateway_err(h_fp,"dump_vgs: wring HTML headr",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }

                      if(l_vars->hdf_path_r != NULL)
                          fprintf(h_fp,"%s Vgroup%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                                  (ntvg > 1 ? "These" : "This"),(ntvg > 1 ? "s" : ""),
                                  l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
                      else
                          fprintf(h_fp,"%s Vgroup%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                                  (ntvg > 1 ? "These" : "This"),(ntvg > 1 ? "s" : ""),
                                  l_vars->cgi_path,l_vars->f_path_r,l_vars->f_name,l_vars->f_name);
                  }
                else
                    h_fp = l_vars->hfp;

                fprintf(h_fp, "<H2>Vgroups</H2><P>\n");
                fprintf(h_fp, "The top level has the following vgroup(s).<P>\n"); 
                if (!l_vars->do_dump) 
                  {
                      fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                      fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                              l_vars->h_env->path_info);
                      fprintf(h_fp, "<SELECT NAME=\"Vgroups\" SIZE=5>\n");
                  }

                /* process throught toplevel Vgroups */
                for (i=0; i < ntvg; i++)
                  {
                      if ((tvgelem = next_in_list(*(l_vars->tvg_list))) == NULL)
                        {
                            ret_value = FAIL;
                            goto done;
                        }
                      DBUG_PRINT(3,(LOGF,"tvgelem->vref= %d \n",tvgelem->vref));
                      DBUG_PRINT(3,(LOGF,"tvgelem->class= %s \n",tvgelem->class));
                      if (!HDstrncmp(tvgelem->class,"CDF0.0",HDstrlen("CDF0.0"))
                          || !HDstrncmp(tvgelem->class,"Attr0.0",HDstrlen("Attr0.0"))
                          || !HDstrncmp(tvgelem->class,"Var0.0",HDstrlen("Var0.0"))
                          || !HDstrncmp(tvgelem->class,"Dim0.0",HDstrlen("Dim0.0"))
                          || !HDstrncmp(tvgelem->class,"UDim0.0",HDstrlen("UDim0.0"))
                          || !HDstrncmp(tvgelem->class,"DimVal0.0",HDstrlen("DimVal0.0"))
                          || !HDstrncmp(tvgelem->class,"Data0.0",HDstrlen("Data0.0")))
                        {
                            continue;
                        }

                      if (i==0)
                          fprintf(h_fp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> ", 
                                  DFTAG_VG, tvgelem->vref, 0);
                      else
                          fprintf(h_fp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ", 
                                  DFTAG_VG, tvgelem->vref, 0);
                      fprintf(h_fp, "%s\n", tvgelem->name);
                  } /* end for i */
                
                if(l_vars->do_dump)
                    fclose(h_fp);

                if (!l_vars->do_dump)
                  {
                      fprintf(l_vars->hfp, "</SELECT> <P>");
                      fprintf(l_vars->hfp, "To select a particular element, press this button: ");
                      fprintf(l_vars->hfp, "<INPUT TYPE=\"submit\" VALUE=\"Select Element\">. <P>\n");
                      fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",
                              l_vars->f_name); 
                      fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                              2);
                      fprintf(l_vars->hfp, "</FORM>");
                  }
            } /* end ntvg */
      } /* end if vg->list */
        
  done:

    EXIT(2,"dump_vgs");
    return ret_value;
} /* dump_vgs */

/*-------------------------------------------------------------------- 
 NAME
     show_vgtree - prints Vgroup and elements
 DESCRIPTION
     Prints Vgroup and elements. Recurses
 RETURNS
     SUCCEED/FAIL     
--------------------------------------------------------------------*/ 
intn 
show_vgtree(tvg_stp  vg_elem, /* Vgroup element to print */
            int32 length,     /* current line length to print */
            lvar_st *l_vars) 
{
    int32     j, k;
    tvg_stp   nvg_elem = NULL;         /* pointer to Vgroup element */
    telem_st  *el_elem = NULL;
    char      *name = NULL;
    intn ret_value = SUCCEED;

    ENTER(2,"show_vgtree");    

    /* prepare for this Vgroup */
    fprintf(l_vars->hfp, 
            "<A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">Vgroup(%s)</A>", 
            l_vars->h_env->script_name,l_vars->h_env->path_info, l_vars->f_name,
            DFTAG_VG, vg_elem->vref, 0, vg_elem->name);

    DBUG_PRINT(3,(LOGF,"show_vgtree: processing Vgroup %s\n",
                  vg_elem->name));

    /* reset the list */
    if (vg_elem->nelem)
        reset_to_beginning(*(vg_elem->elem_list));
    /* process the children of this Vgroup */
    for (j = 0; j < vg_elem->nelem ; j++)
      {
          el_elem = next_in_list(*(vg_elem->elem_list));

          /* get ready on correct line */
          fprintf(l_vars->hfp, "-- ");
          DBUG_PRINT(3,(LOGF,"show_vgtree:processing element tag=%d, ref=%d \n",el_elem->tag, el_elem->ref));
          /* Check to see if it is a Vgroup */
          switch (el_elem->tag)
            {
            case DFTAG_VG:
                DBUG_PRINT(3,(LOGF,"show_vgtree:found non-toplevel Vgroup \n"));
                /* need to recurse  on this Vgroup */
                reset_to_beginning(*(l_vars->ovg_list));
                nvg_elem = first_that(*(l_vars->ovg_list),vg_cmp,&(el_elem->ref));
                if(nvg_elem != NULL)
                  {
                      DBUG_PRINT(3,(LOGF,"show_vgtree: name %s\n",
                                    nvg_elem->name));
                      if (show_vgtree(nvg_elem,length,l_vars) == FAIL)
                        {
                            ret_value = FAIL;
                            goto done;
                        }
                  }
                break;
            case DFTAG_NDG:
                /* print HTML ref */
                fprintf(l_vars->hfp,
                        "<A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">SDS(%s)</A>", 
                        l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                        el_elem->tag, el_elem->ref, 0, el_elem->name);

                break;
            case DFTAG_VH:
                /* print HTML ref */
                fprintf(l_vars->hfp,
                        "<A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">Vdata(%s)</A>", 
                        l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                        el_elem->tag, el_elem->ref, 0, el_elem->name);

                break;
            default:
                /* print HTML ref */
                if (el_elem->name != NULL)
                  {
                      fprintf(l_vars->hfp,
                              "<A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">Object %s</A>", 
                              l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                              el_elem->tag, el_elem->ref, 0, el_elem->name);
                  }
                else
                  {
                      /* get name of element */
                      if ((name = HDgettagsname((uint16) el_elem->tag)) == NULL)
                          name = (char *)strdup("Unknown Tag");

                      /* print HTML ref */
                      fprintf(l_vars->hfp,
                              "<A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">Object %s</A>", 
                              l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                              el_elem->tag, el_elem->ref, 0, name);

                      /* free up tmp name space */
                      HDfree(name);
                  }
                break;
            }

          fprintf(l_vars->hfp, "\n");
          for (k = 0; k < length+7; k++)
              fprintf(l_vars->hfp, " ");
      } /* end for element count */

    fprintf(l_vars->hfp, "   ");

  done:

    EXIT(2,"show_vgtree");
    return ret_value;
} /* show_vgtree */

/*-------------------------------------------------------------------- 
 NAME
     show_tree - prints a tree representation of the objects in the HDF file
 DESCRIPTION
     This routine prints a grhapical tree representation of the objects in the
     HDF file 
 RETURNS
     SUCCEED/FAIL     
--------------------------------------------------------------------*/ 
intn 
show_tree(char *fname,    /* file name */
          lvar_st *l_vars) 
{
    int32    i;
    intn     ntvg = 0;
    int32    length =0;
    tvg_stp  tvg_elem = NULL;         /* pointer to Vgroup element */
    intn ret_value = SUCCEED;

    ENTER(2,"show_tree");
    /* print toplevel Vgroups */
    if (l_vars->tvg_list != NULL)
      {   /* number of toplevel Vgroups */
          ntvg = num_of_objects(*(l_vars->tvg_list)); 

          /* toplelvel nodes */
          if (ntvg)
              fprintf(l_vars->hfp, "<hr>\n <h3>Graphical representation of Toplevel Vgroups in the file:</h3>\n");

          /* process throught toplevel Vgroups */
          reset_to_beginning(*(l_vars->tvg_list));
          for (i = 0; i < ntvg; i++)
            {
                /* find next toplevel Vgroup to process */
                if ((tvg_elem = next_in_list(*(l_vars->tvg_list))) == NULL)
                  {
                      ret_value = FAIL;
                      goto done;
                  }

                length = HDstrlen(tvg_elem->name)+1;
                /* process this Vgroup */
                if (show_vgtree(tvg_elem,length,l_vars) == FAIL)
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                    
                fprintf(l_vars->hfp, "\n");
            } /* end for top level Vgroups */
      }

  done:

    EXIT(2,"show_tree");
    return ret_value;
} /* show_tree */

