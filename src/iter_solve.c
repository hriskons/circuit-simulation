#include "iter_solve.h"
#include "linear_helper.h"

#include <math.h>

#define ABS(value)  ( (value) >=0 ? (value) : -(value) )

static int iter = 10;
static double tolerance = 1e-3;


void iter_set_options( int iterations , double itol ){
	iter = iterations;
	tolerance = itol;
}

static void print_matrix_gsl(gsl_matrix* A)
{
	int i,j;

	for(i = 0; i < A->size1; i++)
	{
		for(j = 0; j < A->size2; j++)
		{
			printf("\t%.4f\t",gsl_matrix_get(A,i,j));
		}
		printf("\n");
	}
}

static void print_vector_gsl(gsl_vector* A)
{
	int i;

	for(i = 0; i < A->size; i++)
	{
		printf("\t%.4f\n",gsl_vector_get(A,i));

	}
}

gsl_vector* iter_solve_cg(gsl_matrix* A , gsl_vector* b , gsl_vector* x0 ){

	int iteration = 0 ;

	gsl_vector* r;
	r = gsl_vector_calloc(b->size);
	if( !r )
		return NULL;


	gsl_vector* b1;
	b1 = gsl_vector_calloc(b->size);
	if( !b1)
		return NULL; 

	gsl_vector* z;
	z = gsl_vector_calloc(b->size);
	if( !z)
		return NULL;

	gsl_vector* p;
	p = gsl_vector_calloc(b->size);
	if( !p )
		return NULL;

	gsl_vector* q;
	q = gsl_vector_calloc(b->size);
	if( !q )
		return NULL;

	gsl_vector* M = lh_get_inv_diag(A);
	//print_vector_gsl(M);

	if( !M ){
		gsl_vector_free(r);
		gsl_vector_free(b1);
		gsl_vector_free(z);
		gsl_vector_free(p);
		
		gsl_vector_free(q);
		return NULL;
	}

	gsl_vector_memcpy(b1,b);
	
	/* r = b - Ax */
	lh_matrix_vector_mul_and_sum( x0,A,b1,NON_TRANSP,-1.0,1.0);
	
	gsl_vector_memcpy(r , b1);
	gsl_vector_free(b1);

	double rho,rho1;
	double beta;
	double alpha;

	while ( iteration < iter && ( (lh_norm(r) / lh_norm(b)) > tolerance ) ){

		iteration++;

		lh_diag_mul(z,r,M);	// Solve Mz = r
	
		rho = lh_dot_product( r , z);
		
		if( iteration == 1 ){
			gsl_vector_memcpy(p,z); 	// p = z 
		}
		else{
			beta = rho / rho1;

			//gsl_vector* temp_v = gsl_vector_calloc(p->size);
			
			/* p = z + beta*p */
			lh_scalar_vector_mul(p, beta,p); //  p = beta* p
			
			gsl_vector_add( p , z);			 //  p = z + p
			
		}
		
		rho1 = rho;
		if(iteration == 2)
		{
			print_vector_gsl(p);
			printf("\n");
			print_vector_gsl(q);
			printf("\n");		
		}
		printf("\n");
		/* q = Ap */
		lh_matrix_vector_mul( p,A,q,NON_TRANSP);
		if(iteration == 2)
		{
			print_vector_gsl(p);
			printf("\n");
			print_vector_gsl(q);
		}
		alpha = rho / lh_dot_product( p , q);
		
		/* x = x + alpha * p */
		gsl_vector* temp_v = gsl_vector_calloc(p->size);
		if( !temp_v ){
			gsl_vector_free(r);
			gsl_vector_free(b1);
			gsl_vector_free(z);
			gsl_vector_free(p);
		
			gsl_vector_free(q);
			return NULL;
		}
		lh_scalar_vector_mul(temp_v , alpha , p); // temp_v = alha * p
		
		gsl_vector_add( x0 , temp_v);			  // x = x + temp_v
		
		/* r = r - alpha * q */
		//gsl_vector_memcpy(temp_v , q);
		lh_scalar_vector_mul( temp_v , alpha , q); // temp_v = alpha* p
		
		gsl_vector_sub(r,temp_v);				   // r = r - temp_v
		
		gsl_vector_free(temp_v);
	}


	/* clean up */
	gsl_vector_free(r);
	gsl_vector_free(z);
	gsl_vector_free(p);
	gsl_vector_free(M);	
	gsl_vector_free(q);

	/* result written in x0 */
	return x0;
}

gsl_vector* iter_solve_bicg(gsl_matrix* A , gsl_vector* b , gsl_vector* x0 ){
	int i = 0;
	double alpha = 0;
	double beta = 0;
	double rho = 0;
	double rho_1 = 1;
	double norm_r = 0;
	double norm_b = 0;
	double omega = 0;
	double eps = pow(10,-14);

	gsl_vector* M;

	gsl_vector* r;
	r = gsl_vector_calloc(A->size1);

	gsl_vector* r_t;
	r_t = gsl_vector_calloc(A->size1);

	gsl_vector* z;
	z = gsl_vector_calloc(b->size);

	gsl_vector *z_t;
	z_t = gsl_vector_calloc(b->size);

	gsl_vector *temp_z;
	temp_z = gsl_vector_calloc(b->size);

	gsl_vector *temp_z_t;
	temp_z_t = gsl_vector_calloc(b->size);

	gsl_vector* p;
	p = gsl_vector_calloc(b->size);

	gsl_vector *p_t;
	p_t = gsl_vector_calloc(b->size);

	gsl_vector* q;
	q = gsl_vector_calloc(b->size);

	gsl_vector *q_t;
	q_t = gsl_vector_calloc(b->size);

	if(p == NULL || p_t == NULL || r == NULL || r_t == NULL || z == NULL || z_t == NULL || temp_z == NULL || temp_z_t == NULL || q == NULL || q_t == NULL)
	{
		gsl_vector_free(r);
		gsl_vector_free(r_t);
		gsl_vector_free(q);
		gsl_vector_free(q_t);
		gsl_vector_free(z);
		gsl_vector_free(z_t);
		gsl_vector_free(temp_z);
		gsl_vector_free(temp_z_t);

		perror("Allocation failed... I am going to exit now");
		exit (1);
	}
	
	print_matrix_gsl(A);
	/* r = r~ = b - Ax */
	lh_matrix_vector_mul_and_sum(x0,A,b,NON_TRANSP,-1.0,1.0);

	/* r = b */
	gsl_vector_memcpy(r,b);
	/* ~r = b */
	gsl_vector_memcpy(r_t,b);


	/* Get and save the 1/diag(A) in the vector m */
	M = lh_get_inv_diag(A);

	/* get the euclidian norms of r and b vectors */
	norm_r = lh_norm(r);
	norm_b = lh_norm(b);

	while(( norm_r / norm_b ) > tolerance && i < iter)
	{
		i++;
		lh_diag_mul(z,r,M); 	/* 	Preconditioner solve*/
		lh_diag_mul(z_t,r_t,M);	/*	Transpose prec-solve */


		rho = lh_dot_product(z,r_t);

		if(ABS(rho) < eps) 		/* Algorithm failure */
		{
			printf("rho =  %lf eps: %f\n",rho,eps);
			perror("Algorithm failed in iter_solve_bicg ---> rho");
			exit(1);
		}

		if (iter == 1)
		{
			gsl_vector_memcpy(p,z); 	/* p = z */
			gsl_vector_memcpy(p_t,z_t); /* p~ = z~ */
		}
		else
		{
			beta = rho / rho_1;
			
			gsl_vector_memcpy(temp_z,z); 		/* 	temp_z = z */
			gsl_vector_memcpy(temp_z_t, z_t);	/*	temp_z_t = z_t */
			
			lh_scalar_vector_mul(p,beta,p);		/* p = beta * p where beta scalar and p vector*/
			lh_scalar_vector_mul(p_t,beta,p_t);	/* p~ = beta * p~ where beta scalar and p vector*/
			gsl_vector_add (temp_z,p);			/* temp_z = temp_z + p */
			gsl_vector_add(temp_z_t,p_t);		/* temp_z~ = temp_z~ + p~ */
			
			gsl_vector_memcpy(p,temp_z);		/* p = z + beta * p */
			gsl_vector_memcpy(p_t,temp_z);		/* p~ = z~ + beta * p~ */
		}
		rho_1 = rho;

		lh_matrix_vector_mul( p, A, q ,NON_TRANSP); /* q = Ap */
		lh_matrix_vector_mul( p_t, A, q_t ,TRANSP); /* q~ = transposed(A)p~*/

		
		omega = lh_dot_product(p_t,q);
		if(ABS(omega) < eps)
		{
			perror("Algorithm failed in iter_solve_bicg ----> omega");
			exit(1);
		}

		alpha = rho / omega;

		lh_scalar_vector_mul(p,alpha,p);
		gsl_vector_add (x0,p);
		lh_scalar_vector_mul(q,alpha,q);
		gsl_vector_sub (r,q);
		lh_scalar_vector_mul(q_t,alpha,q_t);
		gsl_vector_sub (r_t,q_t);
	}
	

	return x0;
}

